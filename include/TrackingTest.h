#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "CommandTest.h"
#include "MapiHandler.h"
#include "utils.h"

class TrackingTest {
  protected:
    std::string m_testName;
    std::shared_ptr<MapiHandler> m_mapi;
    double m_expectedInterval;
    size_t m_maxLineLength;
    std::string m_pattern;
    ValueValidator m_valueValidator;

    std::thread m_worker;
    std::atomic<bool> m_running;
    std::atomic<bool> m_stopFlag;

    utils::Welford m_stats;

    void loop();

    virtual void logSummary() const {
        return;
    }

  public:
    TrackingTest(
        std::string testName,
        std::shared_ptr<MapiHandler> mapi,
        double expectedInterval,
        size_t maxLineLength,
        std::string pattern,
        ValueValidator valueValidator
    );

    void start(double expectedInterval = -1.);
    void stop(bool doLogSummary = true);

    TrackingTest(TrackingTest&& other) noexcept :
        m_testName(std::move(other.m_testName)),
        m_mapi(std::move(other.m_mapi)),
        m_expectedInterval(other.m_expectedInterval),
        m_maxLineLength(other.m_maxLineLength),
        m_pattern(std::move(other.m_pattern)),
        m_valueValidator(std::move(other.m_valueValidator)),
        m_worker(std::move(other.m_worker)), 
        m_running(other.m_running.load()),
        m_stopFlag(other.m_stopFlag.load()),
        m_stats(std::move(other.m_stats)) {}

    TrackingTest& operator=(TrackingTest&& other) noexcept {
        if (this != &other) {
            m_testName = std::move(other.m_testName);
            m_mapi = std::move(other.m_mapi);
            m_expectedInterval = other.m_expectedInterval;
            m_maxLineLength = other.m_maxLineLength;
            m_pattern = std::move(other.m_pattern);
            m_valueValidator = std::move(other.m_valueValidator);
            m_worker = std::move(other.m_worker);
            m_running = other.m_running.load();
            m_stopFlag = other.m_stopFlag.load();
            m_stats = std::move(other.m_stats);
        }
        return *this;
    }

    TrackingTest(const TrackingTest&) = delete;
    TrackingTest& operator=(const TrackingTest&) = delete;

    virtual ~TrackingTest() = default;
};
