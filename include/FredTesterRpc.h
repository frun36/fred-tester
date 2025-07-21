#pragma once

#include <condition_variable>
#include <optional>

#include "Result.h"
#include "dis.hxx"
#include "tests/TesterConfig.h"

class FredTesterRpc: public DimRpc {
  private:
    std::condition_variable m_cv;
    std::mutex m_mtx;
    std::optional<tests::TesterConfig> m_cfg;

    void rpcHandler() override;

  public:
    Result<void> waitAndExecute();

    FredTesterRpc(const char* name = "FRED_TESTER/CONTROL") :
        DimRpc(name, "C", "C") {}
};
