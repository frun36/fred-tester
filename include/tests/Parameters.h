#pragma once

#include "CommandTest.h"
#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Parameters: public TestSuite {
  private:
    CommandTest writeLSB15(utils::Board board);
    CommandTest writeElectronicMSB255(utils::Board board);
    CommandTest doubleRead(
        utils::Board board,
        std::pair<double, double> expected
    );
    CommandTest writeOutOfRangeLSB(utils::Board board);
    CommandTest readNonexistent(utils::Board board);
    CommandTest doubleWrite(
        utils::Board board,
        std::pair<double, double> values
    );
    CommandTest doubleWriteOutOfRangeMSB(utils::Board board);

  public:
    Parameters(utils::Board board);
};

} // namespace tests
