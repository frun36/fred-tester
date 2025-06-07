#pragma once

#include <format>
#include <iomanip>
#include <iostream>
#include <mutex>

class Logger {
  private:
    std::mutex m_mtx;

    static Logger& getInstance() {
        static Logger logger;
        return logger;
    }

    void print(const std::string& s) {
        std::lock_guard<std::mutex> lock(m_mtx);

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()
                  )
            % 1000;
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
        localtime_r(&in_time_t, &buf);

        std::cout << "[" << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << '.'
                  << std::setw(3) << std::setfill('0') << ms.count() << "] "
                  << s << '\n';
    }

    static void log(std::string s) {
        getInstance().print(s);
    }

  public:
    template<typename... Args>
    static void debug(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "\033[34mD\033[0m [{}] {}",
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
                "\033[32mI\033[0m [{}] {}",
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
                "\033[33mW\033[0m [{}] {}",
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
                "\033[31mE\033[0m [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }
};
