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
};
