#pragma once

#include <format>
#include <iostream>

class Logger {
  private:
    static Logger& getInstance() {
        static Logger logger;
        return logger;
    }

    void print(const std::string& s) const {
        std::cout << s << '\n';
    }

  public:
    static void log(std::string s) {
        getInstance().print(s);
    }

    template<typename... Args>
    static void debug(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "\033[34mD [{}] {}\033[0m",
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
                "\033[32mI [{}] {}\033[0m",
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
                "\033[33mW [{}] {}\033[0m",
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
                "\033[31mE [{}] {}\033[0m",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }
};
