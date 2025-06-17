#pragma once

#include "Test.h"
#include "utils.h"

namespace tests {

class ResetSystem: public Test {
  public:
    ResetSystem() :
        Test(TestBuilder("RESET_SYSTEM")
                 .mapiName(utils::topic(utils::TCM, "RESET_SYSTEM"))
                 .command("")
                 .pattern(R"(SUCCESS)")
                 .withoutValueValidator()
                 .timeout(3.0)
                 .expectOk()
                 .build()) {}
};

} // namespace tests
