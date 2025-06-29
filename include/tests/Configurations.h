#pragma once

#include "CommandTest.h"
#include "utils.h"

namespace tests {

class Configurations: public CommandTest {
  public:
    Configurations(std::string configurationName) :
        CommandTest(TestBuilder("APPLY CONFIGURATION")
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
