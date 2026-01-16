#pragma once

#include <chrono>
#include <list>
#include <thread>

#include "CommandTest.h"

class TestSuite {
  private:
    std::list<CommandTest> m_tests;
    std::chrono::duration<double> m_sleep;

  public:
    TestSuite(
        std::list<CommandTest> tests,
        std::chrono::duration<double> sleep =
            std::chrono::duration<double>::zero()
    ) :
        m_tests(std::move(tests)),
        m_sleep(sleep) {}

    void run() {
        for (auto& test : m_tests) {
            test.runAndLog();
            std::this_thread::sleep_for(m_sleep);
        }
    }
};
