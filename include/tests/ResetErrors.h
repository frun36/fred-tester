#pragma once

#include "CommandTest.h"
#include "utils.h"

namespace tests {

class ResetErrors: public CommandTest {
  public:
    ResetErrors() :
        CommandTest(TestBuilder("RESET_ERRORS")
                 .mapiName(utils::topic(utils::TCM, "RESET_ERRORS"))
                 .command("")
                 .pattern(R"(SUCCESS)")
                 .withoutValueValidator()
                 .timeout(1.0)
                 .expectOk()
                 .build()) {}
};

} // namespace tests
