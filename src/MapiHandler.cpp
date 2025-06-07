#include "MapiHandler.h"

std::unordered_map<std::string, std::shared_ptr<MapiHandler>>
    MapiHandler::Handlers;

// memory leak after strdup, not a big deal here
MapiHandler::MapiInfo::MapiInfo(
    std::string name,
    CommonResult& res,
    bool isError
) :
    DimUpdatedInfo(strdup(name.c_str()), -1),
    name(name),
    res(res),
    isError(isError) {}

void MapiHandler::MapiInfo::infoHandler() {
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

MapiHandler::MapiHandler(const std::string& name) :
    m_name(name),
    m_req(name + "_REQ"),
    m_ans(name + "_ANS", m_res, false),
    m_err(name + "_ERR", m_res, true) {}

std::expected<std::string, std::string> MapiHandler::handleResponse(
    double timeout,
    bool expectError
) {
    std::unique_lock<std::mutex> lock(m_res.mtx);
    if (!m_res.cv.wait_for(lock, std::chrono::duration<double>(timeout), [&] {
            return m_res.isReady;
        })) {
        return std::unexpected("RESPONSE_TIMEOUT");
    }

    if ((m_res.isError && expectError) || (!m_res.isError && !expectError))
        return m_res.contents;
    return std::unexpected(m_res.contents);
}

std::expected<std::string, std::string> MapiHandler::handleCommandWithResponse(
    std::string command,
    double timeout,
    bool expectError
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

    return handleResponse(timeout, expectError);
}

std::shared_ptr<MapiHandler> MapiHandler::get(const std::string& mapiName) {
    if (!Handlers.contains(mapiName)) {
        Handlers.insert_or_assign(
            mapiName,
            std::make_shared<MapiHandler>(mapiName)
        );
    }
    return Handlers.at(mapiName);
}
