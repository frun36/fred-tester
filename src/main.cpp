#include <regex>
#include <string>
#include <thread>

#include "Logger.h"
#include "MapiHandler.h"

int main(void) {
    MapiHandler topic("FRED/TCM/TCM0/PARAMETERS");

    {
        auto res =
            topic.handle_command("LASER_PATTERN_LSB,WRITE,0xFF", 1., false);
        if (res) {
            Logger::info("Laser pattern write", "{}", *res);
            std::string pat = R"(LASER_PATTERN_LSB,(\d+(\.\d+)?)\n)";
            std::regex re(pat);
            std::smatch match;
            if (!std::regex_match(*res, match, re)) {
                Logger::error(
                    "Laser pattern write",
                    "{} doesn't match {}",
                    *res,
                    pat
                );
            } else {
                double val = std::stod(match[1].str());
                if (val != 255.) {
                    Logger::error("Laser pattern write", "Invalid value");
                }
            }
        }
    }

    {
        auto res = topic.handle_command("LASER_PATTERN_LSB,READ", 1., false);
        if (res) {
            Logger::info("Laser pattern read", "{}", *res);
            std::string pat = R"(LASER_PATTERN_LSB,(\d+(\.\d+)?)\n)";
            std::regex re(pat);
            std::smatch match;
            if (!std::regex_match(*res, match, re)) {
                Logger::error(
                    "Laser pattern read",
                    "{} doesn't match {}",
                    *res,
                    pat
                );
            } else {
                double val = std::stod(match[1].str());
                if (val != 255.) {
                    Logger::error("Laser pattern read", "Invalid value");
                }
            }
        }
    }
}
