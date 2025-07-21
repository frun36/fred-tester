#pragma once

#include <format>
#include <mutex>
#include <optional>

#include "dis.hxx"

class Logger {
  private:
    enum class LogLevel { Debug, Info, Warn, Error };

    std::string_view logLevelStr(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::Debug:
                return "\033[34mDEBUG\033[0m";
            case LogLevel::Info:
                return "\033[32mINFO \033[0m";
            case LogLevel::Warn:
                return "\033[33mWARN \033[0m";
            case LogLevel::Error:
                return "\033[31mERROR\033[0m";
            default:
                return "*****";
        }
    }

    std::mutex m_mtx;
    bool m_debug = false;
    std::optional<DimService> dim = std::nullopt;

    static Logger& getInstance();
    std::string getTimestamp();
    void log(LogLevel lvl, const std::string& name, const std::string& msg);

  public:
    static void setDebug(bool debug);

    static void initDim();

    template<typename... Args>
    static void debug(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        if (!getInstance().m_debug) {
            return;
        }

        getInstance().log(
            LogLevel::Debug,
            name,
            std::format(fmt, std::forward<Args>(args)...)
        );
    }

    template<typename... Args>
    static void info(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        getInstance().log(
            LogLevel::Info,
            name,
            std::format(fmt, std::forward<Args>(args)...)
        );
    }

    template<typename... Args>
    static void warn(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        getInstance().log(
            LogLevel::Warn,
            name,
            std::format(fmt, std::forward<Args>(args)...)
        );
    }

    template<typename... Args>
    static void error(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        getInstance().log(
            LogLevel::Error,
            name,
            std::format(fmt, std::forward<Args>(args)...)
        );
    }
};
