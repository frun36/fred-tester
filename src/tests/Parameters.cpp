#include "tests/Parameters.h"

#include <regex>

#include "Test.h"

namespace tests {

Test Parameters::laserPatternWrite =
    TestBuilder("Laser pattern write")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,WRITE,0x0F")
        .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
        .withValueValidator([](const std::smatch& match) {
            return std::stod(match[1]) == 15;
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test Parameters::laserPatternRead1 =
    TestBuilder("Laser pattern read")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,READ")
        .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
        .withValueValidator([](const std::smatch& match) {
            return std::stod(match[1]) == 15;
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test Parameters::laserPatternWriteElectronic =
    TestBuilder("Laser pattern write electronic")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,WRITE,0xFF")
        .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
        .withValueValidator([](const std::smatch& match) {
            return std::stod(match[1]) == 255;
        })
        .timeout(0.1)
        .expectOk()
        .build();

Test Parameters::laserPatternRead2 =
    TestBuilder("Laser pattern read")
        .mapiName("FRED/TCM/TCM0/PARAMETERS")
        .command("LASER_PATTERN_MSB,READ")
        .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
        .withValueValidator([](const std::smatch& match) {
            return std::stod(match[1]) == 255;
        })
        .timeout(0.1)
        .expectOk()
        .build();

} // namespace tests
