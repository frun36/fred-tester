#include "tests/Configurations.h"

#include "utils.h"

namespace tests {
Configurations::Configurations(std::string configurationName) :
    CommandTest(
        TestBuilder("APPLY CONFIGURATION")
            .mapiName(utils::topic(utils::TCM0, "CONFIGURATIONS"))
            .command(configurationName)
            .pattern(R"({}\n(?:(?:TCM0|PM[AC][0-9]),OK\n)+)", configurationName)
            .withoutValueValidator()
            .timeout(1.0)
            .expectOk()
            .build()
    ) {}

} // namespace tests
