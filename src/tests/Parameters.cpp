#include "tests/Parameters.h"

#include <string>

#include "Test.h"

namespace tests {

Test TcmParameters::laserPatternWrite =
    TestBuilder("Laser pattern write")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,WRITE,0x0F")
        .pattern(R"(LASER_PATTERN_MSB,({})\n)", FLT)
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
    TestBuilder("Laser pattern read")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,READ")
        .pattern(R"(LASER_PATTERN_MSB,({})\n)", FLT)
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
    TestBuilder("Laser pattern write electronic")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,WRITE,0xFF")
        .pattern(R"(LASER_PATTERN_MSB,({})\n)", FLT)
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
    TestBuilder("Laser pattern read")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,READ")
        .pattern(R"(LASER_PATTERN_MSB,({})\n)", FLT)
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
