#include "Test.h"
#include "TestSuite.h"

namespace tests {

class Parameters: public TestSuite {
    static Test laserPatternWrite;
    static Test laserPatternRead1;
    static Test laserPatternWriteElectronic;
    static Test laserPatternRead2;

  public:
    Parameters() :
        TestSuite(
            {laserPatternWrite,
             laserPatternRead1,
             laserPatternWriteElectronic,
             laserPatternRead2}
        ) {}
};

} // namespace tests
