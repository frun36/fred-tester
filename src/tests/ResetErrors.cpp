#include "tests/ResetErrors.h"

#include "utils.h"

namespace tests {

ResetErrors::ResetErrors() :
    CommandTest(TestBuilder("RESET_ERRORS")
                    .mapiName(utils::topic(utils::TCM0, "RESET_ERRORS"))
                    .command("")
                    .pattern(R"(SUCCESS)")
                    .withoutValueValidator()
                    .timeout(1.0)
                    .expectOk()
                    .build()) {}
} // namespace tests
