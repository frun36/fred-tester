#include <regex>
#include <string>

#include "Logger.h"
#include "MapiHandler.h"
#include "Test.h"

int main(void) {
    auto validator0 = [](const std::smatch& match) {
        return std::stod(match[1].str()) == 0x0F;
    };
    auto validator1 = [](const std::smatch& match) {
        return std::stod(match[1].str()) == 0xFF;
    };
    Test t0 = TestBuilder("Laser pattern write")
                  .mapiName("FRED/TCM/TCM0/PARAMETERS")
                  .command("LASER_PATTERN_MSB,WRITE,0x0F")
                  .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
                  .withValueValidator(validator0)
                  .timeout(0.1)
                  .expectOk()
                  .build();

    Test t1 = TestBuilder("Laser pattern read")
                  .mapiName("FRED/TCM/TCM0/PARAMETERS")
                  .command("LASER_PATTERN_MSB,READ")
                  .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
                  .withValueValidator(validator0)
                  .timeout(0.1)
                  .expectOk()
                  .build();

    Test t2 = TestBuilder("Laser pattern write electronic")
                  .mapiName("FRED/TCM/TCM0/PARAMETERS")
                  .command("LASER_PATTERN_MSB,WRITE,0xFF")
                  .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
                  .withValueValidator(validator1)
                  .timeout(0.1)
                  .expectOk()
                  .build();

    Test t3 = TestBuilder("Laser pattern read")
                  .mapiName("FRED/TCM/TCM0/PARAMETERS")
                  .command("LASER_PATTERN_MSB,READ")
                  .pattern(R"(LASER_PATTERN_MSB,{}\n)", FLT)
                  .withValueValidator(validator1)
                  .timeout(0.1)
                  .expectOk()
                  .build();

    t0.run();
    t1.run();
}
