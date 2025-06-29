#pragma once

#include <list>

#include "CommandTest.h"

class TestSuite {
  private:
    std::list<CommandTest> m_tests;

  public:
    TestSuite(std::list<CommandTest> m_tests) : m_tests(std::move(m_tests)) {}

    void run() {
        for (auto& test : m_tests) {
            test.runAndLog();
        }
    }
};
