#include "tests/Parameters.h"

#include <string>

#include "Test.h"
#include "utils.h"

using namespace utils;

namespace tests {

Test TcmParameters::laserPatternWrite =
    TestBuilder("Write")
        .mapiName(Topic("TCM0", "PARAMETERS"))
        .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0x0F")
        .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)", FLT)
        .withValueValidator([](auto match) -> Result<void> {
            if (std::stod(match[1]) == 15) {
                return {};
            } else {
                return Error("Read value {}", match[1].str());
            }
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test TcmParameters::laserPatternRead1 =
    TestBuilder("Read")
        .mapiName(Topic("TCM0", "PARAMETERS"))
        .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,READ")
        .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)", FLT)
        .withValueValidator([](auto match) -> Result<void> {
            if (std::stod(match[1]) == 15) {
                return {};
            } else {
                return Error("Read value {}", match[1].str());
            }
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test TcmParameters::laserPatternWriteElectronic =
    TestBuilder("Write electronic")
        .mapiName(Topic("TCM0", "PARAMETERS"))
        .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0xFF")
        .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)", FLT)
        .withValueValidator([](auto match) -> Result<void> {
            if (std::stod(match[1]) == 255) {
                return {};
            } else {
                return Error("Read value {}", match[1].str());
            }
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test TcmParameters::laserPatternRead2 =
    TestBuilder("Read")
        .mapiName(Topic("TCM0", "PARAMETERS"))
        .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,READ")
        .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)", FLT)
        .withValueValidator([](auto match) -> Result<void> {
            if (std::stod(match[1]) == 255) {
                return {};
            } else {
                return Error("Read value {}", match[1].str());
            }
        })
        .timeout(0.1)
        .expectOk()
        .build();

} // namespace tests
