#pragma once

#include <ranges>
#include "Result.h"
#include "utils.h"

struct ParameterValidator {
    struct Val {
        double value;
        bool eq;
    };

    std::unordered_map<std::string, Val> expectedValues;

    ParameterValidator& eq(std::string name, double value) {
        expectedValues[name] = {value, true};
        return *this;
    }

    ParameterValidator& neq(std::string name, double value) {
        expectedValues[name] = {value, false};
        return *this;
    }

    Result<void> validate(std::string str) {
        std::unordered_map<std::string, double> statusValues;
        statusValues.reserve(1024);
        std::string errorReport;
        for (auto line : std::views::split(str, '\n')) {
            if (line.empty())
                continue;
            auto parts = std::views::split(line, ',');
            auto it = parts.begin();
            std::string_view name(*it++);
            std::string_view valueStr(*it++);
            auto valueRes = utils::parseDouble(std::string(valueStr));
            if (!valueRes) {
                errorReport += std::format("{}: {}\n", name, valueRes.error());
            }

            statusValues[std::string(name)] = *valueRes;
        }

        for (auto [name, expected] : expectedValues) {
            auto statusIt = statusValues.find(name);
            if (statusIt == statusValues.end())
                errorReport += std::format(
                    "Parameter {} not found in STATUS response\n",
                    name
                );

            double statusValue = statusIt->second;
            if ((statusValue == expected.value) != expected.eq)
                errorReport += std::format(
                    "{} = {}; expected {} {}\n",
                    name,
                    statusValue,
                    expected.eq ? "=" : "≠",
                    expected.value
                );
        }

        if (errorReport.empty()) {
            return {};
        } else {
            errorReport.pop_back();
            return err("{}", errorReport);
        }
    }
};
