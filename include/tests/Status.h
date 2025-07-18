#include <format>

#include "MapiHandler.h"
#include "TrackingTest.h"
#include "utils.h"

namespace tests {

class Status: public TrackingTest {
  public:
    Status(utils::Board board) :
        TrackingTest(
            board.name() + " STATUS TRACKER",
            MapiHandler::get(topic(board, "STATUS")),
            1.0,
            0,
            std::format(
                R"((?:(?!IS_BOARD_OK,){},{}\n)*IS_BOARD_OK,({})\n)", // only matching group - after IS_BOARD_OK
                utils::STR,
                utils::FLT,
                utils::FLT
            ),
            [](auto match) -> Result<void> {
                double val = std::stod(match[1]);
                if (val != 1.)
                    return err("IS_BOARD_OK = {}", val);
                else
                    return {};
            }
        ) {}

    Status(Status&&) = default;
    Status& operator=(Status&&) = default;

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
};

} // namespace tests
