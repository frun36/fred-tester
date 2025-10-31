#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Parameters: public TestSuite {
  private:
    struct ParameterOperation {
        std::string name;
        std::string type;
        double value;
        bool validation = false;
    };

    CommandTest generateTestOk(
        utils::Board board,
        std::string testName,
        std::vector<ParameterOperation> operations
    );
    CommandTest generateTestErr(
        utils::Board board,
        std::string testName,
        std::vector<ParameterOperation> operations,
        std::string errorMsg
    );

  public:
    Parameters(utils::Board board);
};

} // namespace tests
