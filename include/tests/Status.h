#include <format>

#include "MapiHandler.h"
#include "Test.h"
#include "TrackingTest.h"

namespace tests {

class Status: public TrackingTest {
  public:
    Status() :
        TrackingTest(
            "STATUS tracker",
            MapiHandler::get("FRED/TCM/TCM0/STATUS"),
            1.0,
            std::format(
                R"((?:(?!IS_BOARD_OK,){},{}\n)*IS_BOARD_OK,({})\n(?:{},{}\n)*)", // only matching group - after IS_BOARD_OK
                STR,
                FLT,
                FLT,
                STR,
                FLT
            ),
            [](auto match) -> Result<void> {
                double val = std::stod(match[1]);
                if (val != 1.)
                    return {};
                else
                    return Error("IS_BOARD_OK = {}", val);
            }
        ) {}
};

} // namespace tests
