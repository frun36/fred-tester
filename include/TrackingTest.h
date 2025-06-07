#pragma once

#include <atomic>
#include <memory>
#include <regex>
#include <string>

#include "MapiHandler.h"
#include "Test.h"

class TrackingTest {
  private:
    std::string testName;
    std::shared_ptr<MapiHandler> mapi;
    double expectedInterval;
    std::string pattern;
    ValueValidator valueValidator;

    std::thread worker;
    std::atomic<bool> running;
    std::atomic<bool> stopFlag;

    size_t count = 0;
    double mean = 0;
    double m2 = 0;
    double stddev = 0;
    void resetStats();
    void updateStats(double elapsed);

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
