#pragma once

#include "TestSuite.h"

namespace tests {

class Configurations: public TestSuite {
  public:
    Configurations(std::string configurationName) :
        TestSuite({TestBuilder("Apply configuration")
                       .mapiName(Topic("TCM0", "CONFIGURATIONS"))
                       .command(configurationName)
                       .pattern("OK")
                       .withoutValueValidator()
                       .timeout(1.0)
                       .expectOk()
                       .build()}) {}
};

} // namespace tests
