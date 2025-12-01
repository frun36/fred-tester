#include <unordered_map>

#include "TrackingTest.h"
#include "utils.h"
#include "ParameterValidator.h"

namespace tests {

class Status: public TrackingTest {
  private:
    ParameterValidator m_validator;

  public:
    Status(utils::Board board, std::vector<utils::Board> connectedBoards);

    Status(Status&& other);
    Status& operator=(Status&& other);

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
};

} // namespace tests
