#pragma once

#include "Test.h"
#include "utils.h"

namespace tests {

class ResetErrors: public Test {
  public:
    ResetErrors() :
        Test(TestBuilder("RESET_ERRORS")
                 .mapiName(utils::topic(utils::TCM, "RESET_ERRORS"))
                 .command("")
                 .pattern(R"(SUCCESS)")
                 .withoutValueValidator()
                 .timeout(1.0)
                 .expectOk()
                 .build()) {}
};

} // namespace tests
