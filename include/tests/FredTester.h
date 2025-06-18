#pragma once

#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"

namespace tests {

class FredTester {
  private:
    Status tcmStatus {utils::TCM};
    Status pmStatus {utils::PM};
    CounterRates tcmCounterRates {utils::TCM};
    CounterRates pmCounterRates {utils::PM};

  public:
    bool setup();
    void changeReadInterval();
    void run();
    void histograms();
    void finish();
};

} // namespace tests
