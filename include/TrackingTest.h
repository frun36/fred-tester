#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "MapiHandler.h"
#include "Test.h"
#include "utils.h"

class TrackingTest {
  protected:
    std::string m_testName;
    std::shared_ptr<MapiHandler> m_mapi;
    double m_expectedInterval;
    std::string m_pattern;
    ValueValidator m_valueValidator;

    std::thread m_worker;
    std::atomic<bool> m_running;
    std::atomic<bool> m_stopFlag;

    utils::Welford m_stats;

    void loop();

  public:
    TrackingTest(
        std::string testName,
        std::shared_ptr<MapiHandler> mapi,
        double expectedInterval,
        std::string pattern,
        ValueValidator valueValidator
    );

    void start();
    void stop();
};
