#include "Logger.h"

void Logger::setDebug(bool debug) {
    getInstance().m_debug = debug;
}

void Logger::initDim(DimService* dim) {
    getInstance().dim = dim;
}

void Logger::log(
    LogLevel lvl,
    const std::string& name,
    const std::string& msg
) {
    std::lock_guard<std::mutex> lock(m_mtx);

    std::string timestamp = getTimestamp();
    std::string logLine =
        std::format("{} {} [{}] {}\n", timestamp, logLevelStr(lvl), name, msg);

    switch (lvl) {
        case LogLevel::Debug:
        case LogLevel::Info:
            std::cout << logLine;
            break;
        case LogLevel::Warn:
        case LogLevel::Error:
            std::cerr << logLine;
            break;
    }

    if (dim) {
        // std::cout << "Updating DIM service\n";
        dim->updateService(const_cast<char*>(logLine.c_str()));
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
              )
        % 1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    localtime_r(&in_time_t, &buf);

    return std::format(
        "\033[2m{:4}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}\033[0m",
        buf.tm_year + 1900,
        buf.tm_mon + 1,
        buf.tm_mday,
        buf.tm_hour,
        buf.tm_min,
        buf.tm_sec,
        ms.count()
    );
}

Logger& Logger::getInstance() {
    static Logger logger;
    return logger;
}
