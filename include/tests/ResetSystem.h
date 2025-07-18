#pragma once

#include "CommandTest.h"
#include "utils.h"

namespace tests {

class ResetSystem: public CommandTest {
  public:
    ResetSystem() :
        CommandTest(TestBuilder("RESET_SYSTEM")
                 .mapiName(utils::topic(utils::TCM0, "RESET_SYSTEM"))
                 .command("")
                 .pattern(R"(SUCCESS)")
                 .withoutValueValidator()
                 .timeout(3.0)
                 .expectOk()
                 .build()) {}
};

} // namespace tests
