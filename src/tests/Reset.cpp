#include "tests/Reset.h"

#include <ranges>

#include "utils.h"

namespace tests {

std::list<CommandTest> Reset::generateTests() {
    std::list<std::string> names = {
        "GBT_RESET",
        "GBT_RESET_DATA_COUNTERS",
        "GBT_START_OF_EMULATION",
        "GBT_RESET_ORBIT_SYNC",
        "GBT_RESET_READOUT_FSM",
        "GBT_RESET_RX_ERROR",
        "GBT_RESET_RX_PHASE_ERROR",
        "GBT_RESET_ERROR_REPORT_FIFO",
    };

    auto tests = names | std::views::transform([](auto name) {
                     return TestBuilder("{}", name)
                         .mapiName(utils::topic(utils::TCM0, "RESET"))
                         .command(name)
                         .pattern(R"(SUCCESS)")
                         .withoutValueValidator()
                         .timeout(1.0)
                         .expectOk()
                         .build();
                 });

    return std::list<CommandTest>(tests.begin(), tests.end());
}

Reset::Reset() : TestSuite(generateTests(), std::chrono::milliseconds(500)) {}

} // namespace tests
