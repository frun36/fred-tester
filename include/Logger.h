#pragma once

#include <format>
#include <iostream>

class Logger {
  private:
    static Logger& get_instance() {
        static Logger logger;
        return logger;
    }

    void print(const std::string& s) const {
        std::cout << s << '\n';
    }

  public:
    static void log(std::string s) {
        get_instance().print(s);
    }

    template<typename... Args>
    static std::string debug(
        const std::string& name,
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        return log(
            std::format(
                "D [{}] {}",
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
                "I [{}] {}",
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
                "W [{}] {}",
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
                "E [{}] {}",
                name,
                std::format(fmt, std::forward<Args>(args)...)
            )
        );
    }
};
