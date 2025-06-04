#pragma once

#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>

#include "Logger.h"
#include "MapiHandler.h"

static constexpr std::string_view FLT = R"(([+-]?(?:\d+\.\d*|\.\d+|\d+)))";
static constexpr std::string_view DEC = R"(([+-]?\d+))";
static constexpr std::string_view HEX = R"(([+-]?0[xX][0-9a-fA-F]+))";
static constexpr std::string_view STR = R"(([^,]+))";

struct Test {
    static std::unordered_map<std::string, std::shared_ptr<MapiHandler>>
        Handlers;

    Test(
        std::string testName,
        std::shared_ptr<MapiHandler> mapi,
        std::string command,
        double timeout,
        bool isError,
        std::string pattern,
        std::function<bool(const std::smatch&)> valueValidator
    ) :
        testName(std::move(testName)),
        mapi(mapi),
        command(std::move(command)),
        timeout(timeout),
        isError(isError),
        pattern(std::move(pattern)),
        valueValidator(std::move(valueValidator)) {}

    std::string testName;
    std::shared_ptr<MapiHandler> mapi;
    std::string command;
    double timeout;
    bool isError;
    std::string pattern;
    std::function<bool(const std::smatch&)> valueValidator;

    void run() {
        auto response = mapi->handleCommand(command, timeout, isError);

        if (!response) {
            Logger::error(
                testName,
                "Unexpected {}: {}",
                (isError ? "success" : "error"),
                response.error()
            );
            return;
        }

        std::regex re(pattern);
        std::smatch match;

        if (!std::regex_match(*response, match, re)) {
            Logger::error(testName, "Invalid response");
        }

        if (valueValidator != nullptr) {
            if (!valueValidator(match)) {
                Logger::error(testName, "Invalid value in response");
            }
        }

        Logger::info(testName, "Success");
    }
};

class TestBuilder {
    std::string m_name = "";
    std::string m_mapiName = "";
    std::string m_command = "";
    double m_timeout = 0.1;
    bool m_expectError = false;
    std::string m_pattern = "";
    std::function<bool(const std::smatch&)> m_valueValidator = nullptr;

  public:
    TestBuilder(std::string name) : m_name(name) {}

    TestBuilder& mapiName(std::string mapiName) {
        m_mapiName = std::move(mapiName);
        return *this;
    }

    TestBuilder& command(std::string command) {
        m_command = std::move(command);
        return *this;
    }

    TestBuilder& timeout(double timeout) {
        m_timeout = timeout;
        return *this;
    }

    TestBuilder& expectError() {
        m_expectError = true;
        return *this;
    }

    TestBuilder& expectOk() {
        m_expectError = false;
        return *this;
    }

    template<typename... Args>
    TestBuilder&
    pattern(const std::format_string<Args...>& fmt, Args&&... args) {
        m_pattern = std::format(fmt, args...);
        return *this;
    }

    TestBuilder&
    withValueValidator(std::function<bool(const std::smatch&)> valueValidator) {
        m_valueValidator = valueValidator;
        return *this;
    }

    TestBuilder& withoutValueValidator() {
        m_valueValidator = nullptr;
        return *this;
    }

    Test build() const {
        if (!Test::Handlers.contains(m_mapiName)) {
            Test::Handlers.insert_or_assign(
                m_mapiName,
                std::make_shared<MapiHandler>(m_mapiName)
            );
        }
        std::shared_ptr<MapiHandler> mapiHandler =
            Test::Handlers.at(m_mapiName);
        return Test(
            m_name,
            mapiHandler,
            m_command,
            m_timeout,
            m_expectError,
            m_pattern,
            m_valueValidator
        );
    }
};
