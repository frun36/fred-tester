#pragma once

#include "TestSuite.h"

namespace tests {

class Reset: public TestSuite {
    static std::list<CommandTest> generateTests();
    
  public:
    Reset();
};

} // namespace tests
