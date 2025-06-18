#pragma once

#include <functional>
#include <memory>
#include <regex>
#include <string>

#include "MapiHandler.h"
#include "Result.h"

using ValueValidator = std::function<Result<void>(std::smatch)>;

struct Test {
    Test(
        std::string testName,
        std::shared_ptr<MapiHandler> mapi,
        std::string command,
        double timeout,
        bool isError,
        std::string pattern,
        ValueValidator valueValidator
    );

    std::string testName;
    std::shared_ptr<MapiHandler> mapi;
    std::string command;
    double timeout;
    bool isError;
    std::string pattern;
    ValueValidator valueValidator;

    Result<void> run();
    bool runAndLog();
};

class TestBuilder {
    std::string m_name = "";
    std::string m_mapiName = "";
    std::string m_command = "";
    double m_timeout = 0.1;
    bool m_expectError = false;
    std::string m_pattern = "";
    ValueValidator m_valueValidator = nullptr;

  public:
    template<typename... Args>
    TestBuilder(const std::format_string<Args...>& fmt, Args&&... args) :
        m_name(std::format(fmt, std::forward<Args>(args)...)) {}

    TestBuilder& mapiName(std::string mapiName);
    TestBuilder& command(std::string command);
    TestBuilder& timeout(double timeout);
    TestBuilder& expectError();
    TestBuilder& expectOk();

    template<typename... Args>
    TestBuilder& pattern(
        const std::format_string<Args...>& fmt,
        Args&&... args
    ) {
        m_pattern = std::format(fmt, std::forward<Args>(args)...);
        return *this;
    }

    TestBuilder& withValueValidator(ValueValidator valueValidator);
    TestBuilder& withoutValueValidator();
    Test build() const;
};
