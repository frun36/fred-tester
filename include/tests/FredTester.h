#pragma once

#include <utility>

#include "dis.hxx"
#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"
#include "tests/TesterConfig.h"
#include "utils.h"

namespace tests {

class FredTester {
  private:
    TesterConfig m_cfg;
    std::vector<std::pair<utils::Board, Status>> status;
    std::vector<std::pair<utils::Board, CounterRates>> counterRates;

    DimService* m_badChannelMap = nullptr;
    void publishBadChannelMap(std::string map);

  public:
    FredTester(TesterConfig cfg, DimService* badChannelMap = nullptr);

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
