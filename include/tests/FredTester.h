#pragma once

#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"
#include "tests/TesterConfig.h"

namespace tests {

class FredTester {
  private:
    TesterConfig cfg;
    Status tcmStatus {utils::TCM};
    Status pmStatus {utils::PM};
    CounterRates tcmCounterRates {utils::TCM};
    CounterRates pmCounterRates {utils::PM};

  public:
    FredTester(TesterConfig cfg) : cfg(cfg) {}

    bool setup();
    void changeReadInterval();
    void resetReadInterval();
    void run();
    void tcmHistograms();
    void pmHistograms();
    void cleanup();
    void finish();
};

} // namespace tests
