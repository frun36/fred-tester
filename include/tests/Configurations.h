#pragma once

#include "Test.h"
#include "utils.h"

namespace tests {

class Configurations: public Test {
  public:
    Configurations(std::string configurationName) :
        Test(TestBuilder("APPLY CONFIGURATION")
                 .mapiName(utils::topic("TCM0", "CONFIGURATIONS"))
                 .command(configurationName)
                 .pattern(
                     R"({}\n(?:(?:TCM0|PM[AC][0-9]),OK\n)+)",
                     configurationName
                 )
                 .withoutValueValidator()
                 .timeout(1.0)
                 .expectOk()
                 .build()) {}
};

} // namespace tests
