#pragma once

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <optional>
#include <string>

#include "Logger.h"
#include "dic.hxx"

class TopicHandler {
  private:
    struct CommonResult {
        std::mutex mtx;
        std::condition_variable cv;
        std::string message;
        bool is_ready;
        bool is_error;
        bool is_handled;
    };

    struct MapiInfo: public DimUpdatedInfo {
        // memory leak after strdup, not a big deal here
        MapiInfo(std::string name, CommonResult& res, bool is_error) :
            DimUpdatedInfo(strdup(name.c_str()), -1),
            name(name),
            res(res),
            is_error(is_error) {}

        std::string name;
        CommonResult& res;
        bool is_error;

        void infoHandler() override {
            std::lock_guard<std::mutex> lock(res.mtx);
            if (res.is_handled) {
                Logger::warning(name, "Unexpected response");
                return;
            }
            res.message = getString();
            res.is_ready = true;
            res.is_error = is_error;
            res.is_handled = true;
            res.cv.notify_one();
        }
    };

    std::string m_name;
    CommonResult m_res;
    std::string m_req;
    MapiInfo m_ans;
    MapiInfo m_err;

  public:
    TopicHandler(const std::string& name) :
        m_name(name),
        m_req(name + "_REQ"),
        m_ans(name + "_ANS", m_res, false),
        m_err(name + "_ERR", m_res, true) {}

    std::optional<std::string> handle_command(
        std::string command,
        double timeout,
        bool expect_error = false
    ) {
        {
            std::lock_guard<std::mutex> lock(m_res.mtx);
            m_res.is_ready = false;
            m_res.is_handled = false;
            m_res.message.clear();
            m_res.is_error = false;
        }
        DimClient::sendCommand(m_req.c_str(), command.c_str());

        std::unique_lock<std::mutex> lock(m_res.mtx);
        if (!m_res.cv.wait_for(
                lock,
                std::chrono::duration<double>(timeout),
                [&] { return m_res.is_ready; }
            )) {
            Logger::error(m_name, "Response timeout");
            return std::nullopt;
        }

        if (m_res.is_error && expect_error || !m_res.is_error && !expect_error)
            return m_res.message;
        Logger::error(m_name, "{}", m_res.message);
        return std::nullopt;
    }
};
