#include "Test.h"

#include "Logger.h"
#include "utils.h"

Test::Test(
    std::string testName,
    std::shared_ptr<MapiHandler> mapi,
    std::string command,
    double timeout,
    bool isError,
    std::string pattern,
    ValueValidator valueValidator
) :
    testName(std::move(testName)),
    mapi(mapi),
    command(std::move(command)),
    timeout(timeout),
    isError(isError),
    pattern(std::move(pattern)),
    valueValidator(std::move(valueValidator)) {}

void Test::run() {
    auto response = mapi->handleCommandWithResponse(command, timeout, isError);

    if (!response) {
        if (response.error() == "RESPONSE_TIMEOUT") {
            Logger::error(testName, "Timeout when waiting for response");
        } else {
            Logger::error(
                testName,
                "Unexpected {}: {}",
                (isError ? "success" : "error"),
                utils::shorten(response.error())
            );
        }
        return;
    }

    std::regex re(pattern);
    std::smatch match;

    if (!std::regex_match(*response, match, re)) {
        Logger::error(
            testName,
            "Invalid response: {}",
            utils::shorten(*response)
        );
    }

    if (valueValidator != nullptr) {
        auto val = valueValidator(std::move(match));
        if (!val) {
            Logger::error(testName, "Value validation failed: {}", val.error());
        }
    }

    Logger::info(testName, "Success");
}

TestBuilder& TestBuilder::mapiName(std::string mapiName) {
    m_mapiName = std::move(mapiName);
    return *this;
}

TestBuilder& TestBuilder::command(std::string command) {
    m_command = std::move(command);
    return *this;
}

TestBuilder& TestBuilder::timeout(double timeout) {
    m_timeout = timeout;
    return *this;
}

TestBuilder& TestBuilder::expectError() {
    m_expectError = true;
    return *this;
}

TestBuilder& TestBuilder::expectOk() {
    m_expectError = false;
    return *this;
}

TestBuilder& TestBuilder::withValueValidator(ValueValidator valueValidator) {
    m_valueValidator = valueValidator;
    return *this;
}

TestBuilder& TestBuilder::withoutValueValidator() {
    m_valueValidator = nullptr;
    return *this;
}

Test TestBuilder::build() const {
    return Test(
        m_name,
        MapiHandler::get(m_mapiName),
        m_command,
        m_timeout,
        m_expectError,
        m_pattern,
        m_valueValidator
    );
}
