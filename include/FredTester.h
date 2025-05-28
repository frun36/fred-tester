#pragma once

#include <string>
#include <vector>

class FredTester {
    std::vector<std::string> m_tests;

  public:
    explicit FredTester(std::vector<std::string> m_tests) :
        m_tests(std::move(m_tests)) {}

    void run();
};
