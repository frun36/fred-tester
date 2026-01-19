#include "tests/HistogramsTracking.h"

#include "tests/HistogramsSingle.h"

namespace tests {
TcmHistogramsTracking::TcmHistogramsTracking(bool selectableHistogramEnabled) :
    TrackingTest(
        "TCM0 HISTOGRAMS TRACKING",
        MapiHandler::get(utils::topic(utils::TCM0, "HISTOGRAMS")),
        1.0,
        128,
        std::format(
            R"(({})\n)"
            R"(SELECTABLE{}(?:\.\.\.)?\n)"
            R"(01,[0-9,]+(?:\.\.\.)?\n)"
            R"(02,[0-9,]+(?:\.\.\.)?\n)"
            R"(READ_ELAPSED,({})ms\n)"
            R"(PREV_ELAPSED,({})ms\n)"
            R"((Executed:.*)?)",
            utils::HEX,
            selectableHistogramEnabled ? ",[0-9,]+" : "",
            utils::FLT,
            utils::FLT
        ),
        nullptr
    ) {}

PmHistogramsTracking::PmHistogramsTracking(
    utils::Board board,
    bool adc0,
    bool adc1,
    bool time
) :
    TrackingTest(
        std::format("{} HISTOGRAMS TRACKING", board.name()),
        MapiHandler::get(utils::topic(board, "HISTOGRAMS")),
        1.0,
        128,
        std::format(
            R"(({})\n)"
            R"({})"
            R"(READ_ELAPSED,({})ms\n)"
            R"(PREV_ELAPSED,({})ms\n)"
            R"((Executed:.*)?)",
            utils::HEX,
            pmHistRegex(adc0, adc1, time),
            utils::FLT,
            utils::FLT
        ),
        nullptr
    ) {}
} // namespace tests
