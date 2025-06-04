#pragma once

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <expected>
#include <mutex>
#include <optional>
#include <string>

#include "Logger.h"
#include "dic.hxx"

class MapiHandler {
  private:
    struct CommonResult {
        std::mutex mtx;
        std::condition_variable cv;
        std::string contents;
        bool isReady;
        bool isError;
        bool isHandled;
    };

    struct MapiInfo: public DimUpdatedInfo {
        // memory leak after strdup, not a big deal here
        MapiInfo(std::string name, CommonResult& res, bool isError) :
            DimUpdatedInfo(strdup(name.c_str()), -1),
            name(name),
            res(res),
            isError(isError) {}

        std::string name;
        CommonResult& res;
        bool isError;

        void infoHandler() override {
            std::lock_guard<std::mutex> lock(res.mtx);
            Logger::debug(name, "Received: {}", getString());
            if (res.isHandled) {
                Logger::warning(name, "Unexpected response: {}", getString());
                return;
            }
            res.contents = getString();
            res.isReady = true;
            res.isError = isError;
            res.isHandled = true;
            res.cv.notify_one();
        }
    };

    std::string m_name;
    CommonResult m_res;
    std::string m_req;
    MapiInfo m_ans;
    MapiInfo m_err;

  public:
    MapiHandler(const std::string& name) :
        m_name(name),
        m_req(name + "_REQ"),
        m_ans(name + "_ANS", m_res, false),
        m_err(name + "_ERR", m_res, true) {}

    std::expected<std::string, std::string> handleCommand(
        std::string command,
        double timeout,
        bool expectError = false
    ) {
        {
            std::lock_guard<std::mutex> lock(m_res.mtx);
            m_res.isReady = false;
            m_res.isHandled = false;
            m_res.contents.clear();
            m_res.isError = false;
        }
        DimClient::sendCommand(m_req.c_str(), command.c_str());

        Logger::debug(m_name, "Sent command: {}", command);

        std::unique_lock<std::mutex> lock(m_res.mtx);
        if (!m_res.cv.wait_for(
                lock,
                std::chrono::duration<double>(timeout),
                [&] { return m_res.isReady; }
            )) {
            return std::unexpected("Response timeout");
        }

        if ((m_res.isError && expectError)
            || (!m_res.isError && !expectError))
            return m_res.contents;
        return std::unexpected(m_res.contents);
    }
};
