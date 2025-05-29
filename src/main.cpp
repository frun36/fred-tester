#include <regex>
#include <string>

#include "Logger.h"
#include "MapiHandler.h"
#include "Test.h"

int main(void) {
    MapiHandler mapi("FRED/TCM/TCM0/PARAMETERS");
    auto validator1 = [](std::smatch match) {
        return std::stod(match[1].str()) == 0x0F;
    };
    auto validator2 = [](std::smatch match) {
        return std::stod(match[1].str()) == 0xFF;
    };
    Test t0("Laser pattern write 0", mapi, "LASER_PATTERN_MSB,WRITE,0x0F", 1, false, R"(LASER_PATTERN_MSB,(\d+(\.\d+)?)\n)", validator1);
    Test t1("Laser pattern read 0", mapi, "LASER_PATTERN_MSB,READ", 1, false, R"(LASER_PATTERN_MSB,(\d+(\.\d+)?)\n)", validator1);
    Test t2("Laser pattern write 1", mapi, "LASER_PATTERN_MSB,WRITE,0xFF", 1, false, R"(LASER_PATTERN_MSB,(\d+(\.\d+)?)\n)", validator2);
    Test t3("Laser pattern read 1", mapi, "LASER_PATTERN_MSB,READ", 1, false, R"(LASER_PATTERN_MSB,(\d+(\.\d+)?)\n)", validator2);
    t0.run();
    t1.run();
    t2.run();
    t3.run();
}
