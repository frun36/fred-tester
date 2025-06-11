#pragma once

#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Configurations: public TestSuite {
  public:
    Configurations(std::string configurationName) :
        TestSuite({TestBuilder("Apply configuration")
                       .mapiName(utils::topic("TCM0", "CONFIGURATIONS"))
                       .command(configurationName)
                       .pattern(
                           R"({}\n(?:(?:TCM0|PM[AC][0-9]),OK\n)+)",
                           configurationName
                       )
                       .withoutValueValidator()
                       .timeout(1.0)
                       .expectOk()
                       .build()}) {}
};

} // namespace tests
