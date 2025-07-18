#pragma once

#include <utility>

#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"
#include "tests/TesterConfig.h"
#include "utils.h"

namespace tests {

class FredTester {
  private:
    TesterConfig cfg;
    std::vector<std::pair<utils::Board, Status>> status;
    std::vector<std::pair<utils::Board, CounterRates>> counterRates;

  public:
    FredTester(TesterConfig cfg);

    bool setup();
    void changeReadInterval();
    void resetReadInterval();
    void run();
    void tcmHistograms();
    void pmHistograms(utils::Board board);
    void cleanup();
    void finish();
};

} // namespace tests
