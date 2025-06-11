#pragma once

#include <array>
#include <chrono>

#include "Result.h"
#include "TrackingTest.h"
#include "utils.h"

namespace tests {

class TcmCounterRates: public TrackingTest {
    struct Response {
        Response() = default;

        Response(
            std::string readIntervalState,
            double readIntervalSeconds,
            std::string fifoState,
            uint32_t fifoLoad,
            std::vector<uint32_t> counters,
            std::vector<double> rates,
            double prevElapsed
        );

        std::string readIntervalState;
        double readIntervalSeconds;
        std::string fifoState;
        uint32_t fifoLoad;
        std::vector<uint32_t> counters;
        std::vector<double> rates;
        double prevElapsed;

        bool hasCounters() const {
            return !counters.empty();
        }

        bool hasRates() const {
            return !rates.empty();
        }

        static Result<Response> fromMatch(std::smatch match);
    };

    struct ValueTracker {
        std::array<utils::Welford, 15> counters;
        std::array<utils::Welford, 15> rates;
        utils::Welford elapsed;
        utils::Welford newValuesInterval;

        std::chrono::time_point<std::chrono::steady_clock> lastTime =
            std::chrono::steady_clock::now();

        Result<void> operator()(std::smatch match);
        std::string summary() const;
    };

    ValueTracker m_valueTracker;

  public:
    TcmCounterRates();

    void logSummary() const;
};

} // namespace tests
