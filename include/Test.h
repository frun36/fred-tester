#pragma once

#include <regex>
#include <string>

#include "Logger.h"
#include "MapiHandler.h"

struct Test {
    Test(
        std::string test_name,
        MapiHandler& mapi,
        std::string command,
        double timeout,
        bool is_error,
        std::string pattern,
        std::function<bool(std::smatch)> value_validator
    ) :
        test_name(std::move(test_name)),
        mapi(mapi),
        command(std::move(command)),
        timeout(timeout),
        is_error(is_error),
        pattern(std::move(pattern)),
        value_validator(std::move(value_validator)) {}

    std::string test_name;
    MapiHandler& mapi;
    std::string command;
    double timeout;
    bool is_error;
    std::string pattern;
    std::function<bool(std::smatch)> value_validator;

    void run() {
        auto response = mapi.handle_command(command, timeout, is_error);

        if (!response) {
            Logger::error(test_name, "Unexpected {}: {}", (is_error ? "success" : "error"), response.error());
            return;
        }

        std::regex re(pattern);
        std::smatch match;

        if (!std::regex_match(*response, match, re)) {
            Logger::error(test_name, "Invalid response");
        }

        if (value_validator != nullptr && !value_validator(std::move(match))) {
            Logger::error(test_name, "Invalid value");
        }

        Logger::info(test_name, "Success");
    }
};
