#pragma once

#include <condition_variable>
#include <optional>

#include "dis.hxx"
#include "tests/TesterConfig.h"

class FredTesterRpc: public DimRpc {
  private:
    std::condition_variable m_cv;
    std::mutex m_mtx;
    std::optional<tests::TesterConfig> m_cfg;

    DimService m_badChannelMap;

    void rpcHandler() override;

  public:
    void waitAndExecute();

    FredTesterRpc(const char* name = "FRED_TESTER/CONTROL") :
        DimRpc(name, "C", "C"), m_badChannelMap("FRED_TESTER/BAD_CHANNEL_MAP", (char*)"") {}
};
