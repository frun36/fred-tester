#pragma once

#include <format>

#include "Test.h"
#include "utils.h"

namespace tests {

class TcmHistogramsSingle: public Test {
  public:
    TcmHistogramsSingle(bool selectableHistogramEnabled) :
        Test(
            TestBuilder(
                "TCM HISTOGRAMS READ{}",
                selectableHistogramEnabled ? " SEL" : ""
            )
                .mapiName(utils::topic(utils::TCM, "HISTOGRAMS"))
                .command("READ")
                .pattern(
                    R"(({})\n)"
                    R"(SELECTABLE{}\n)"
                    R"(01(?:,{})+\n)"
                    R"(02(?:,{})+\n)",
                    utils::HEX,
                    selectableHistogramEnabled
                        ? std::format("(?:,{})+", utils::DEC)
                        : "",
                    utils::DEC,
                    utils::DEC
                )
                .withoutValueValidator()
                .expectOk()
                .timeout(0.2)
                .build()
        ) {}
};

static inline std::string pmHistRegex(std::string histName) {
    std::string re;
    for (size_t i = 0; i < 12; i++) {
        re += std::format(R"(CH{:02}{}(,{})*\n)", i + 1, histName, utils::FLT);
    }
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
                 .mapiName(utils::topic(utils::TCM, "HISTOGRAMS"))
                 .command("READ")
                 .pattern(
                     R"(({})\n)"
                     R"({})",
                     utils::HEX,
                     (adc0 ? pmHistRegex("ADC0") : std::string(""))
                         + (adc1 ? pmHistRegex("ADC1") : std::string(""))
                         + (time ? pmHistRegex("TIME") : std::string(""))
                 )
                 .withoutValueValidator()
                 .expectOk()
                 .timeout(0.2)
                 .build()) {}
};

} // namespace tests
