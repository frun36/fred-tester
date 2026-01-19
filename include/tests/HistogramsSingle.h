#pragma once

#include "CommandTest.h"
#include "utils.h"

namespace tests {

class TcmHistogramsSingle: public CommandTest {
  public:
    TcmHistogramsSingle(bool selectableHistogramEnabled);
};

static inline std::string pmHistRegex(bool adc0, bool adc1, bool time) {
    std::string re;
    re += R"(ADC0)";
    if (adc0) {
        re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
    }
    re += R"(\n)";

    re += R"(ADC1)";
    if (adc1) {
        re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
    }
    re += R"(\n)";

    re += R"(TIME)";
    if (time) {
        re += std::format(R"(,[0-9,]+(?:\.\.\.)?)");
    }
    re += R"(\n)";
    return re;
}

class PmHistogramsSingle: public CommandTest {
  public:
    PmHistogramsSingle(utils::Board board, bool adc0, bool adc1, bool time);
};

} // namespace tests
