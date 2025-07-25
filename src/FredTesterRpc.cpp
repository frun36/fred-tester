#include "FredTesterRpc.h"

#include <optional>

#include "Logger.h"
#include "tests/FredTester.h"

void FredTesterRpc::rpcHandler() {
    Logger::debug("FRED_TESTER", "DIM: {}", getString());
    std::unique_lock<std::mutex> lock(m_mtx, std::defer_lock);
    if (!lock.try_lock()) {
        setData(const_cast<char*>("BUSY"));
        Logger::debug("FRED_TESTER", "BUSY");
        return;
    }

    auto toml = toml::parse(getString());
    if (!toml) {
        Logger::error(
            "FRED_TESTER",
            "TOML parsing error: {}",
            toml.error().description()
        );
        setData(const_cast<char*>("ERROR"));
        return;
    }

    auto cfgRes = tests::TesterConfig::fromToml(toml.table());
    if (!cfgRes) {
        Logger::error(
            "FRED_TESTER",
            "Failed to construct config: {}",
            cfgRes.error()
        );
        return;
    }

    m_cfg.emplace(std::move(*cfgRes));

    setData(const_cast<char*>("OK"));
    m_cv.notify_one();
}

void FredTesterRpc::waitAndExecute() {
    using namespace std::chrono;

    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]() { return m_cfg.has_value(); });
    tests::FredTester tester(*m_cfg, &m_badChannelMap);
    tester.run();
    m_cfg = std::nullopt;
}
