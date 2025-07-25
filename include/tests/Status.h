#include <unordered_map>

#include "TrackingTest.h"
#include "utils.h"

namespace tests {

class Status: public TrackingTest {
  private:
    struct ValueValidator {
        struct Val {
            double value;
            bool eq;
        };

        std::unordered_map<std::string, Val> expectedValues;

        ValueValidator& eq(std::string name, double value);
        ValueValidator& neq(std::string name, double value);

        Result<void> validate(std::string str);
    };

    ValueValidator m_validator;

  public:
    Status(utils::Board board, std::vector<utils::Board> connectedBoards);

    Status(Status&& other);
    Status& operator=(Status&& other);

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
};

} // namespace tests
