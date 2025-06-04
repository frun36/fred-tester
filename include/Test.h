#pragma once

#include <functional>
#include <regex>
#include <string>

#include "Logger.h"
#include "MapiHandler.h"
#include "Pattern.h"

struct Test {
    Test(
        std::string testName,
        MapiHandler& mapi,
        std::string command,
        double timeout,
        bool isError,
        Pattern pattern,
        std::function<bool(void)> valueValidator
    ) :
        testName(std::move(testName)),
        mapi(mapi),
        command(std::move(command)),
        timeout(timeout),
        isError(isError),
        pattern(std::move(pattern)),
        valueValidator(std::move(valueValidator)) {}

    std::string testName;
    MapiHandler& mapi;
    std::string command;
    double timeout;
    bool isError;
    Pattern pattern;
    std::function<bool(void)> valueValidator;

    void run() {
        auto response = mapi.handleCommand(command, timeout, isError);

        if (!response) {
            Logger::error(
                testName,
                "Unexpected {}: {}",
                (isError ? "success" : "error"),
                response.error()
            );
            return;
        }

        if (!pattern.fullMatch(*response)) {
            Logger::error(testName, "Invalid response");
        }

        if (valueValidator != nullptr) {
            pattern.parseValues();
            if (!valueValidator()) {
                Logger::error(testName, "Invalid value in response");
            }
        }

        Logger::info(testName, "Success");
    }
};

struct LaserPattern: public Test {
    double value;

    LaserPattern(MapiHandler& m) :
        Test(
            "LaserPattern",
            m,
            "LASER_PATTERN_MSB,WRITE,0x0F",
            1.0,
            false,
            PatternBuilder("LASER_PATTERN_MSB,{}").flt(&value).build(),
            std::bind(&LaserPattern::valueValidator, this)
        ) {}

    bool valueValidator() const {
        return value == 15;
    }
};
