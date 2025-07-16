#include "TrackingTest.h"

#include <chrono>
#include <stdexcept>

#include "CommandTest.h"
#include "Logger.h"

TrackingTest::TrackingTest(
    std::string testName,
    std::shared_ptr<MapiHandler> mapi,
    double expectedInterval,
    size_t maxLineLength,
    std::string pattern,
    ValueValidator valueValidator
) :
    m_testName(testName),
    m_mapi(mapi),
    m_expectedInterval(expectedInterval),
    m_maxLineLength(maxLineLength),
    m_pattern(pattern),
    m_valueValidator(valueValidator) {
    if (expectedInterval < 0) {
        throw std::runtime_error("Invalid expected interval");
    }

    Logger::debug(
        m_testName,
        "pat {}, expected interval {}s,{} value validator",
        m_pattern,
        m_expectedInterval,
        m_valueValidator == nullptr ? " no" : ""
    );
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

void TrackingTest::stop(bool doLogSummary) {
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

    if (doLogSummary) {
        logSummary();
    }
}

void TrackingTest::loop() {
    std::optional<std::chrono::steady_clock::time_point> lastTime;
    m_stats.reset();
    std::regex re(m_pattern);
    while (!m_stopFlag.load()) {
        auto response = m_mapi->handleResponse(m_expectedInterval * 2);
        if (!response) {
            if (response.error() == "RESPONSE_TIMEOUT") {
                Logger::error(m_testName, "Timeout when waiting for response");
            } else {
                Logger::error(
                    m_testName,
                    "Unexpected error:\n{}",
                    utils::shorten(response.error())
                );
            }
            continue;
        }

        std::string responseStr = m_maxLineLength > 0
            ? utils::shortenLines(*response, m_maxLineLength)
            : std::move(*response);

        std::smatch match;
        if (!std::regex_match(responseStr, match, re)) {
            Logger::error(
                m_testName,
                "Response doesn't match regex\n"
                "{}\n"
                "{}",
                utils::shorten(responseStr),
                utils::shorten(m_pattern)
            );
        }

        if (m_valueValidator != nullptr) {
            auto val = m_valueValidator(std::move(match));
            if (!val) {
                Logger::error(
                    m_testName,
                    "Value validation failed:\n{}",
                    val.error()
                );
            }
        }

        auto now = std::chrono::steady_clock::now();

        if (lastTime) {
            std::chrono::duration<double> elapsed = now - *lastTime;
            m_stats.tick(elapsed.count());

            Logger::debug(
                m_testName,
                "Interval: {:.3f}s | mean {:.3f} | stddev {:.3f}",
                elapsed.count(),
                m_stats.mean(),
                m_stats.stddev()
            );
        }

        lastTime = now;
    }
}
