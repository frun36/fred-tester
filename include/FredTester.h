#pragma once

#include <cstdint>
#include <string>
#include <vector>

class FredTester {
  private:
    std::string m_fred_name;
    std::vector<uint8_t> m_pm_a;
    std::vector<uint8_t> m_pm_c;

  public:
    void run();
};
