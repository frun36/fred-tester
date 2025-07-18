#include "tests/HistogramsSingle.h"

namespace tests {

TcmHistogramsSingle::TcmHistogramsSingle(bool selectableHistogramEnabled) :
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

PmHistogramsSingle::PmHistogramsSingle(
    utils::Board board,
    bool adc0,
    bool adc1,
    bool time
) :
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
} // namespace tests
