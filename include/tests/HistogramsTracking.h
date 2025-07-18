#pragma once

#include "TrackingTest.h"
#include "utils.h"

namespace tests {

class TcmHistogramsTracking: public TrackingTest {
  public:
    TcmHistogramsTracking(bool selectableHistogramEnabled);
};

class PmHistogramsTracking: public TrackingTest {
  public:
    PmHistogramsTracking(utils::Board board, bool adc0, bool adc1, bool time);
};

} // namespace tests
