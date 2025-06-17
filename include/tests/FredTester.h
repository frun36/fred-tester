#pragma once

#include "tests/Configurations.h"
#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/ResetSystem.h"
#include "tests/Status.h"

namespace tests {

class FredTester {
  private:
    ResetSystem resetSystem {};
    Configurations configurations {"laser_1124hz_or_trg"};

    Status tcmStatus {utils::TCM};
    Status pmStatus {utils::PM};
    CounterRates tcmCounterRates {utils::TCM};
    CounterRates pmCounterRates {utils::PM};
    Parameters tcmParameters {utils::TCM};
    Parameters pmParameters {utils::PM};

  public:
    bool setup();
    void changeReadInterval();
    void run();
    void finish();
};

} // namespace tests
