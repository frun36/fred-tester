#include "CommandTest.h"

#include "Logger.h"
#include "utils.h"

CommandTest::CommandTest(
    std::string testName,
    std::shared_ptr<MapiHandler> mapi,
    std::string command,
    double timeout,
    bool isError,
    size_t maxLineLength,
    std::string pattern,
    ValueValidator valueValidator
) :
    testName(std::move(testName)),
    mapi(mapi),
    command(std::move(command)),
    timeout(timeout),
    isError(isError),
    maxLineLength(maxLineLength),
    pattern(std::move(pattern)),
    valueValidator(std::move(valueValidator)) {
    Logger::debug(
        this->testName,
        "cmd {}, pat {}, timeout {}s, isError = {},{} value validator",
        this->command,
        this->pattern,
        this->timeout,
        this->isError,
        this->valueValidator == nullptr ? " no" : ""
    );
}

Result<void> CommandTest::run() {
    auto response = mapi->handleCommandWithResponse(command, timeout, isError);

    if (!response) {
        if (response.error() == "RESPONSE_TIMEOUT") {
            return err("Timeout when waiting for response");
        } else {
            return err(
                "Unexpected {}:\n{}",
                (isError ? "success" : "error"),
                utils::shorten(response.error())
            );
        }
    }

    std::regex re(pattern);
    std::smatch match;

    std::string responseStr = maxLineLength > 0
        ? utils::shortenLines(*response, maxLineLength)
        : std::move(*response);

    if (!std::regex_match(responseStr, match, re)) {
        return err(
            "Response doesn't match regex:\n"
            "{}\n"
            "{}",
            utils::shorten(responseStr),
            utils::shorten(pattern)
        );
    }

    if (valueValidator != nullptr) {
        auto val = valueValidator(std::move(match));
        if (!val) {
            return err("Value validation failed:\n{}", val.error());
        }
    }

    return {};
}

bool CommandTest::runAndLog() {
    auto res = run();
    if (res) {
        Logger::info(testName, "Success");
        return true;
    } else {
        Logger::error(testName, "{}", res.error());
        return false;
    }
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

TestBuilder& TestBuilder::withMaxLineLength(size_t maxLineLength) {
    m_maxLineLength = maxLineLength;
    return *this;
}

TestBuilder& TestBuilder::withoutMaxLineLength() {
    m_maxLineLength = 0;
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

CommandTest TestBuilder::build() const {
    return CommandTest(
        m_name,
        MapiHandler::get(m_mapiName),
        m_command,
        m_timeout,
        m_expectError,
        m_maxLineLength,
        m_pattern,
        m_valueValidator
    );
}
