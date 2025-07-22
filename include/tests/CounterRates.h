#pragma once

#include <chrono>
#include <cstddef>

#include "Result.h"
#include "TesterConfig.h"
#include "TrackingTest.h"
#include "utils.h"

// To test:
// - START
// - STOP
// - RESET
// - change read interval (from parameters)
//
// in TCM histograms:
// - START
// - STOP
// - RESET
// - READ
// - COUNTER (0 or number)
//
// in PM histograms:
// - START
// - STOP
// - RESET
// - READ
// - SELECT
// - HISTOGRAMMING
// - BCID_FILTER

namespace tests {

class CounterRates: public TrackingTest {
    struct Response {
        Response() = default;

        Response(
            std::string readIntervalState,
            double readIntervalSeconds,
            std::string fifoState,
            uint32_t fifoLoad,
            std::string fifoReadResult,
            std::vector<uint32_t> counters,
            std::vector<double> rates,
            double prevElapsed
        );

        std::string readIntervalState;
        double readIntervalSeconds;
        std::string fifoState;
        uint32_t fifoLoad;
        std::string fifoReadResult;
        std::vector<uint32_t> counters;
        std::vector<double> rates;
        double prevElapsed;

        bool hasCounters() const {
            return !counters.empty();
        }

        bool hasRates() const {
            return !rates.empty();
        }

        static Result<Response> fromMatch(
            std::string testName,
            std::smatch match,
            size_t numberOfCounters
        );
    };

    struct ValueTracker {
        std::string testName;
        size_t numberOfCounters;

        double currentInterval;
        std::vector<utils::Welford> rates;
        utils::Welford elapsed;
        utils::Welford newValuesInterval;

        std::optional<std::chrono::time_point<std::chrono::steady_clock>>
            lastTime;

        Result<void> operator()(std::smatch match);

        ValueTracker(
            std::string testName,
            size_t numberOfCounters,
            double currentInterval
        );
    };

    utils::Board m_board;
    ValueTracker m_valueTracker;

    static const std::string TcmPattern;
    static const std::string PmPattern;

    void logSummary() const override;

  public:
    CounterRates(utils::Board board);

    CounterRates(CounterRates&& other) :
        TrackingTest(std::move(other)),
        m_board(std::move(other.m_board)),
        m_valueTracker(std::move(other.m_valueTracker)) {
        m_valueValidator = std::ref(m_valueTracker);
    }

    CounterRates& operator=(CounterRates&& other) {
        if (this != &other) {
            TrackingTest::operator=(std::move(other));
            m_board = std::move(other.m_board);
            m_valueTracker = std::move(other.m_valueTracker);
            m_valueValidator = std::ref(m_valueTracker);
        }
        return *this;
    }

    CounterRates(const CounterRates&) = delete;
    CounterRates& operator=(const CounterRates&) = delete;

    void resetCounters();

    void setInterval(double newInterval) {
        m_valueTracker.currentInterval = newInterval;
    }

    std::string getBadChannelMap(TesterConfig::BadChannelMapConfig cfg);
};

} // namespace tests
