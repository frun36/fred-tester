#pragma once

#include <chrono>
#include <cstddef>

#include "Result.h"
#include "TrackingTest.h"
#include "utils.h"

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

        static Result<Response> fromMatch(std::smatch match, size_t numberOfCounters);
    };

    struct ValueTracker {
        std::string testName;
        size_t numberOfCounters;

        std::vector<utils::Welford> rates;
        utils::Welford elapsed;
        utils::Welford newValuesInterval;

        std::chrono::time_point<std::chrono::steady_clock> lastTime =
            std::chrono::steady_clock::now();

        Result<void> operator()(std::smatch match);

        ValueTracker(std::string testName, size_t numberOfCounters);
    };

    ValueTracker m_valueTracker;
    static const std::string TcmPattern;
    static const std::string PmPattern;

  public:
    CounterRates(std::string boardName);

    void logSummary() const;
};

} // namespace tests
