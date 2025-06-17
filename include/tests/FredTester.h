#pragma once

#include "tests/Configurations.h"
#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"

namespace tests {

class FredTester {
  private:
    static constexpr const char* TCM = "TCM0";
    static constexpr const char* PM = "PMA0";

    Configurations configurations {"laser_1124hz_or_trg"};
    Status tcmStatus {TCM};
    Status pmStatus {PM};
    CounterRates tcmCounterRates {TCM};
    CounterRates pmCounterRates {PM};
    Parameters tcmParameters {TCM};
    Parameters pmParameters {PM};

  public:
    void run();
};

} // namespace tests
