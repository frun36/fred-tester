#include "Test.h"
#include "TestSuite.h"

namespace tests {

class TcmParameters: public TestSuite {
    static Test laserPatternWrite;
    static Test laserPatternRead1;
    static Test laserPatternWriteElectronic;
    static Test laserPatternRead2;

  public:
    TcmParameters() :
        TestSuite(
            {laserPatternWrite,
             laserPatternRead1,
             laserPatternWriteElectronic,
             laserPatternRead2}
        ) {}
};

class PmParameters: public TestSuite {};

} // namespace tests
