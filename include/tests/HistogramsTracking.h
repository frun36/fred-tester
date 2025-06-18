#pragma once

#include "TrackingTest.h"
#include "tests/HistogramsSingle.h"
#include "utils.h"

namespace tests {

class TcmHistogramsTracking: public TrackingTest {
  public:
    TcmHistogramsTracking(bool selectableHistogramEnabled) :
        TrackingTest(
            "TCM HISTOGRAMS TRACKING",
            MapiHandler::get(utils::topic(utils::PM, "HISTOGRAMS")),
            1.0,
            std::format(
                R"(({})\n)"
                R"(SELECTABLE{}\n)"
                R"(01(?:,{})+\n)"
                R"(02(?:,{})+\n)",
                utils::HEX,
                selectableHistogramEnabled ? std::format("(?:,{})+", utils::DEC)
                                           : "",
                utils::DEC,
                utils::DEC
            ),
            nullptr
        ) {}
};

class PmHistogramsTracking: public TrackingTest {
  public:
    PmHistogramsTracking(bool adc0, bool adc1, bool time) :
        TrackingTest(
            "PM HISTOGRAMS TRACKING",
            MapiHandler::get(utils::topic(utils::PM, "HISTOGRAMS")),
            1.0,
            std::format(
                R"(({})\n)"
                R"({})",
                utils::HEX,
                pmHistRegex(adc0, adc1, time)
            ),
            nullptr
        ) {}
};

} // namespace tests
