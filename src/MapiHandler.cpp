#include "MapiHandler.h"

#include <cstring>
#include <unordered_map>

#include "Logger.h"
#include "Result.h"

// memory leak after strdup, not a big deal here
MapiHandler::MapiInfo::MapiInfo(
    const char* name,
    CommonResult& res,
    bool isError
) :
    DimUpdatedInfo(name, -1),
    name(name),
    res(res),
    isError(isError) {}

void MapiHandler::MapiInfo::infoHandler() {
    std::lock_guard<std::mutex> lock(res.mtx);
    if (!res.awaitingResponse) {
        return;
    }

    std::string contents(getString(), getSize() - 1);

    if (res.isReady) {
        Logger::error(
            name,
            "Internal error - awaited response has already been received. Discarding response:\n{}: {}\nCurrent response:\n{}: {}",
            isError ? "_ERR" : "_ANS",
            contents,
            res.isError ? "_ERR" : "_ANS",
            res.contents
        );
        return;
    }
    res.contents = std::move(contents);
    Logger::debug(name, "Received: {}", res.contents);
    res.isReady = true;
    res.isError = isError;
    res.cv.notify_one();
}

MapiHandler::MapiHandler(const std::string& name) :
    m_name(name),
    m_reqName(strdup((name + "_REQ").c_str())),
    m_ansName(strdup((name + "_ANS").c_str())),
    m_errName(strdup((name + "_ERR").c_str())),
    m_req(m_reqName),
    m_ans(m_ansName, m_res, false),
    m_err(m_errName, m_res, true) {}

Result<std::string> MapiHandler::handleResponse(
    double timeout,
    bool expectError
) {
    std::unique_lock<std::mutex> lock(m_res.mtx);
    if (!m_res.cv.wait_for(lock, std::chrono::duration<double>(timeout), [&] {
            return m_res.isReady;
        })) {
        return std::unexpected("RESPONSE_TIMEOUT");
    }
    std::string contents = std::move(m_res.contents);
    bool isError = m_res.isError;

    m_res.reset();
    m_res.awaitingResponse = false;

    if ((isError && expectError) || (!isError && !expectError))
        return contents;
    return std::unexpected(contents);
}

Result<std::string> MapiHandler::handleCommandWithResponse(
    std::string command,
    double timeout,
    bool expectError
) {
    resetResponse();

    sendCommand(command);

    Logger::debug(m_name, "Sent command: {}", command);

    return handleResponse(timeout, expectError);
}

std::shared_ptr<MapiHandler> MapiHandler::get(const std::string& mapiName) {
    static std::unordered_map<std::string, std::shared_ptr<MapiHandler>>
        Handlers;
    if (!Handlers.contains(mapiName)) {
        Handlers.insert_or_assign(
            mapiName,
            std::make_shared<MapiHandler>(mapiName)
        );
    }
    return Handlers.at(mapiName);
}

void MapiHandler::resetResponse() {
    std::unique_lock<std::mutex> lock(m_res.mtx);
    m_res.reset();
    m_res.awaitingResponse = true;
}
