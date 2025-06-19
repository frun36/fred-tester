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
            MapiHandler::get(utils::topic(utils::TCM, "HISTOGRAMS")),
            1.0,
            128,
            std::format(
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
            128,
            std::format(
                R"(({})\n)"
                R"({})"
                R"(READ_ELAPSED,({})ms\n)"
                R"(PREV_ELAPSED,({})ms\n)",
                utils::HEX,
                pmHistRegex(adc0, adc1, time),
                utils::FLT,
                utils::FLT
            ),
            nullptr
        ) {}
};

} // namespace tests
