#include "Test.h"

std::unordered_map<std::string, std::shared_ptr<MapiHandler>> Test::Handlers;

Test::Test(
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

void Test::run() {
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

TestBuilder& TestBuilder::withValueValidator(
    std::function<bool(const std::smatch&)> valueValidator
) {
    m_valueValidator = valueValidator;
    return *this;
}

TestBuilder& TestBuilder::withoutValueValidator() {
    m_valueValidator = nullptr;
    return *this;
}

Test TestBuilder::build() const {
    if (!Test::Handlers.contains(m_mapiName)) {
        Test::Handlers.insert_or_assign(
            m_mapiName,
            std::make_shared<MapiHandler>(m_mapiName)
        );
    }
    std::shared_ptr<MapiHandler> mapiHandler = Test::Handlers.at(m_mapiName);
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
