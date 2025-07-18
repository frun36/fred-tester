#pragma once

#include <format>

#include "Logger.h"
#include "CommandTest.h"
#include "utils.h"

namespace tests {

class TcmHistogramsSingle: public CommandTest {
  public:
    TcmHistogramsSingle(bool selectableHistogramEnabled) :
        CommandTest(TestBuilder(
                 "TCM0 HISTOGRAMS READ{}",
                 selectableHistogramEnabled ? " SEL" : ""
        )
                 .mapiName(utils::topic(utils::TCM0, "HISTOGRAMS"))
                 .command("READ")
                 .withMaxLineLength(128)
                 .pattern(
                     R"(({})\n)"
                     R"(SELECTABLE{}(?:\.\.\.)?\n)"
                     R"(01,[0-9,]+(?:\.\.\.)?\n)"
                     R"(02,[0-9,]+(?:\.\.\.)?\n)"
                     R"(READ_ELAPSED,({})ms\n)"
                     R"(PREV_ELAPSED,({})ms\n)",
                     utils::HEX,
                     selectableHistogramEnabled ? ",[0-9,]+" : "",
                     utils::FLT,
                     utils::FLT
                 )
                 .withoutValueValidator()
                 .expectOk()
                 .timeout(0.5)
                 .build()) {}
};

static inline std::string pmHistRegex(bool adc0, bool adc1, bool time) {
    std::string re;
    for (size_t i = 0; i < 12; i++) {
        re += std::format(R"(CH{:02}ADC0)", i + 1);
        if (adc0) {
            re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
        }
        re += R"(\n)";

        re += std::format(R"(CH{:02}ADC1)", i + 1);
        if (adc1) {
            re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
        }
        re += R"(\n)";

        re += std::format(R"(CH{:02}TIME)", i + 1);
        if (time) {
            re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
        }
        re += R"(\n)";
    }
    return re;
}

class PmHistogramsSingle: public CommandTest {
  public:
    PmHistogramsSingle(utils::Board board, bool adc0, bool adc1, bool time) :
        CommandTest(TestBuilder(
                 "PM HISTOGRAMS READ ({}, {}, {})",
                 adc0 ? std::string("ADC0") : std::string("-"),
                 adc1 ? std::string("ADC1") : std::string("-"),
                 time ? std::string("TIME") : std::string("-")
        )
                 .mapiName(utils::topic(board, "HISTOGRAMS"))
                 .command("READ")
                 .withMaxLineLength(128)
                 .pattern(
                     R"(({})\n)"
                     R"({})"
                     R"(READ_ELAPSED,({})ms\n)"
                     R"(PREV_ELAPSED,({})ms\n)",
                     utils::HEX,
                     pmHistRegex(adc0, adc1, time),
                     utils::FLT,
                     utils::FLT
                 )
                 .withoutValueValidator()
                 .expectOk()
                 .timeout(0.5)
                 .build()) {}
};

} // namespace tests
