#include <format>

#include "MapiHandler.h"
#include "TrackingTest.h"
#include "utils.h"

using namespace utils;

namespace tests {

class Status: public TrackingTest {
  public:
    Status(std::string boardName) :
        TrackingTest(
            boardName + " STATUS tracker",
            MapiHandler::get(topic(boardName, "STATUS")),
            1.0,
            std::format(
                R"((?:(?!IS_BOARD_OK,){},{}\n)*IS_BOARD_OK,({})\n)", // only matching group - after IS_BOARD_OK
                STR,
                FLT,
                FLT
            ),
            [](auto match) -> Result<void> {
                double val = std::stod(match[1]);
                if (val != 1.)
                    return err("IS_BOARD_OK = {}", val);
                else
                    return {};
            }
        ) {}
};

} // namespace tests
