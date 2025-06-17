#include "TrackingTest.h"

#include <chrono>
#include <stdexcept>

#include "Logger.h"
#include "Test.h"

TrackingTest::TrackingTest(
    std::string testName,
    std::shared_ptr<MapiHandler> mapi,
    double expectedInterval,
    std::string pattern,
    ValueValidator valueValidator
) :
    m_testName(testName),
    m_mapi(mapi),
    m_expectedInterval(expectedInterval),
    m_pattern(pattern),
    m_valueValidator(valueValidator) {
    if (expectedInterval < 0) {
        throw std::runtime_error("Invalid expected interval");
    }
}

void TrackingTest::start(double expectedInterval) {
    if (m_running.load())
        return;

    if (expectedInterval > 0) {
        m_expectedInterval = expectedInterval;
    }
    m_stopFlag = false;
    m_worker = std::thread(&TrackingTest::loop, this);
    m_running = true;
    Logger::info(m_testName, "Started tracking");
}

void TrackingTest::stop() {
    if (!m_running.load())
        return;

    m_stopFlag = true;
    if (m_worker.joinable()) {
        m_worker.join();
    }
    m_running = false;
    Logger::info(
        m_testName,
        "Finished tracking: interval {:.3f}±{:.3f}s",
        m_stats.mean(),
        m_stats.stddev()
    );

    logSummary();
}

void TrackingTest::loop() {
    auto lastTime = std::chrono::steady_clock::now();
    m_stats.reset();
    while (!m_stopFlag.load()) {
        auto response = m_mapi->handleResponse(m_expectedInterval * 2);
        if (!response) {
            if (response.error() == "RESPONSE_TIMEOUT") {
                Logger::error(m_testName, "Timeout when waiting for response");
            } else {
                Logger::error(
                    m_testName,
                    "Unexpected error: {}",
                    utils::shorten(response.error())
                );
            }
            continue;
        }

        std::regex re(m_pattern);
        std::smatch match;

        if (!std::regex_match(*response, match, re)) {
            Logger::error(
                m_testName,
                "Invalid response: {}",
                utils::shorten(*response)
            );
        }

        if (m_valueValidator != nullptr) {
            auto val = m_valueValidator(std::move(match));
            if (!val) {
                Logger::error(
                    m_testName,
                    "Value validation failed: {}",
                    val.error()
                );
            }
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        m_stats.tick(elapsed.count());

        Logger::debug(
            m_testName,
            "Interval: {:.3f}s | mean {:.3f} | stddev {:.3f}",
            elapsed.count(),
            m_stats.mean(),
            m_stats.stddev()
        );

        lastTime = now;
    }
}
