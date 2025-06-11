#pragma once

#include <format>
#include <iomanip>
#include <iostream>
#include <mutex>

class Logger {
  private:
    std::mutex m_mtx;
    bool m_debug = false;

    static Logger& getInstance() {
        static Logger logger;
        return logger;
    }

    void print(std::string s) {
        std::lock_guard<std::mutex> lock(m_mtx);

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()
                  )
            % 1000;
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
        localtime_r(&in_time_t, &buf);

        std::cout << "\033[2m" << std::put_time(&buf, "%Y-%m-%d %H:%M:%S")
                  << '.' << std::setw(3) << std::setfill('0') << ms.count()
                  << "\033[0m " << s << '\n';
    }

    static void log(std::string s) {
        getInstance().print(s);
    }

  public:
    static void setDebug(bool debug) {
        getInstance().m_debug = debug;
    }

    template<typename... Args>
    static void debug(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        if (!getInstance().m_debug) {
            return;
        }

        return log(
            std::format(
                "\033[34mDEBUG\033[0m [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }

    template<typename... Args>
    static void info(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "\033[32mINFO \033[0m [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }

    template<typename... Args>
    static void warning(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "\033[33mWARN \033[0m [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }

    template<typename... Args>
    static void error(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "\033[31mERROR\033[0m [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }
};
