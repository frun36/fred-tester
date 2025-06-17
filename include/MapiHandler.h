#pragma once

#include <condition_variable>
#include <expected>
#include <memory>
#include <mutex>
#include <string>

#include "dic.hxx"

class MapiHandler {
  private:
    struct CommonResult {
        std::mutex mtx;
        std::condition_variable cv;
        std::string contents;
        bool isReady = false;
        bool isError = false;

        void reset() {
            isReady = false;
            contents.clear();
            isError = false;
        }
    };

    struct MapiInfo: public DimUpdatedInfo {
        MapiInfo(std::string name, CommonResult& res, bool isError);

        std::string name;
        CommonResult& res;
        bool isError;

        void infoHandler() override;
    };

    std::string m_name;
    CommonResult m_res;
    std::string m_req;
    MapiInfo m_ans;
    MapiInfo m_err;

  public:
    MapiHandler(const std::string& name);

    inline void sendCommand(const std::string& command) {
        DimClient::sendCommand(m_req.c_str(), command.c_str());
    }

    static inline void sendCommand(const std::string& mapiName, const std::string& command) {
        get(mapiName)->sendCommand(command);
    }

    std::expected<std::string, std::string> handleResponse(
        double timeout,
        bool expectError = false
    );

    std::expected<std::string, std::string> handleCommandWithResponse(
        std::string command,
        double timeout,
        bool expectError = false
    );

    static std::shared_ptr<MapiHandler> get(const std::string& mapiName);
};
