#pragma once

#include <format>

#include "Test.h"
#include "Logger.h"
#include "utils.h"

namespace tests {

class TcmHistogramsSingle: public Test {
  public:
    TcmHistogramsSingle(bool selectableHistogramEnabled) :
        Test(TestBuilder(
                 "TCM HISTOGRAMS READ{}",
                 selectableHistogramEnabled ? " SEL" : ""
        )
                 .mapiName(utils::topic(utils::TCM, "HISTOGRAMS"))
                 .command("READ")
                 .pattern(
                     R"(({})\n)"
                     R"(SELECTABLE{}\n)"
                     R"(01(?:,{})+\n)"
                     R"(02(?:,{})+\n)"
                     R"(READ_ELAPSED,({})ms\n)"
                     R"(PREV_ELAPSED,({})ms\n)",
                     utils::HEX,
                     selectableHistogramEnabled
                         ? std::format("(?:,{})+", utils::DEC)
                         : "",
                     utils::DEC,
                     utils::DEC,
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
            re += std::format(R"((,{})*\n)", utils::FLT);
        }

        re += std::format(R"(CH{:02}ADC1)", i + 1);
        if (adc1) {
            re += std::format(R"((,{})*\n)", utils::FLT);
        }

        re += std::format(R"(CH{:02}TIME)", i + 1);
        if (time) {
            re += std::format(R"((,{})*\n)", utils::FLT);
        }
    }
    Logger::warning(re, "");
    return re;
}

class PmHistogramsSingle: public Test {
  public:
    PmHistogramsSingle(bool adc0, bool adc1, bool time) :
        Test(TestBuilder(
                 "PM HISTOGRAMS READ ({}, {}, {})",
                 adc0 ? std::string("ADC0") : std::string("-"),
                 adc1 ? std::string("ADC1") : std::string("-"),
                 time ? std::string("TIME") : std::string("-")
        )
                 .mapiName(utils::topic(utils::PM, "HISTOGRAMS"))
                 .command("READ")
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
