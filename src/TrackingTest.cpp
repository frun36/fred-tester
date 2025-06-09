#include "TrackingTest.h"

#include <chrono>
#include <cmath>

#include "Logger.h"
#include "Test.h"

TrackingTest::TrackingTest(
    std::string testName,
    std::shared_ptr<MapiHandler> mapi,
    double expectedInterval,
    std::string pattern,
    ValueValidator valueValidator
) :
    testName(testName),
    mapi(mapi),
    expectedInterval(expectedInterval),
    pattern(pattern),
    valueValidator(valueValidator) {}

void TrackingTest::start() {
    if (running.load())
        return;

    stopFlag = false;
    worker = std::thread(&TrackingTest::loop, this);
    running = true;
    Logger::info(testName, "Started tracking");
}

void TrackingTest::stop() {
    if (!running.load())
        return;

    stopFlag = true;
    if (worker.joinable()) {
        worker.join();
    }
    running = false;
    Logger::info(
        testName,
        "Finished tracking | mean {:.3f} | stddev {:.3f}",
        mean,
        stddev
    );
}

void TrackingTest::resetStats() {
    count = 0;
    mean = 0.;
    m2 = 0.;
    stddev = 0.;
}

void TrackingTest::updateStats(double elapsed) {
    count++;
    if (count == 1)
        return; // First time measurement is unreliable
    // Welford's algorithm
    double delta = elapsed - mean;
    mean += delta / count;
    m2 += delta * (elapsed - mean);
    stddev = (count > 1) ? std::sqrt(m2 / (count - 1)) : 0.0;
}

void TrackingTest::loop() {
    auto lastTime = std::chrono::steady_clock::now();
    resetStats();
    while (!stopFlag.load()) {
        auto response = mapi->handleResponse(expectedInterval * 2);
        if (!response) {
            if (response.error() == "RESPONSE_TIMEOUT") {
                Logger::error(testName, "Timeout when waiting for response");
            } else {
                Logger::error(
                    testName,
                    "Unexpected error: {}",
                    response.error()
                );
            }
            continue;
        }

        std::regex re(pattern);
        std::smatch match;

        if (!std::regex_match(*response, match, re)) {
            Logger::error(testName, "Invalid response");
        }

        if (valueValidator != nullptr) {
            auto val = valueValidator(std::move(match));
            if (!val) {
                Logger::error(
                    testName,
                    "Value validation failed: {}",
                    val.error()
                );
            }
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        updateStats(elapsed.count());

        Logger::debug(
            testName,
            "Interval: {:.3f}s | mean {:.3f} | stddev {:.3f}",
            elapsed.count(),
            mean,
            stddev
        );

        lastTime = now;
    }
}
