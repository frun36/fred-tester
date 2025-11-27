#include "tests/ResetSystem.h"

#include "utils.h"

namespace tests {

ResetSystem::ResetSystem() :
    CommandTest(TestBuilder("RESET_SYSTEM")
                    .mapiName(utils::topic(utils::TCM0, "RESET_SYSTEM"))
                    .command("")
                    .pattern(R"(SUCCESS)")
                    .withoutValueValidator()
                    .timeout(5.0)
                    .expectOk()
                    .build()) {}
} // namespace tests
