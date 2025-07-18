#pragma once

#include <optional>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

#include "Result.h"
#include "utils.h"

namespace tests {

struct TesterConfig {
    using Boards = std::vector<utils::Board>;

    const Boards connectedBoards;
    const bool resetSystem;
    const bool setupResetErrors;
    const Boards statusTracking;
    const std::optional<std::string> setupConfiguration;
    const Boards counterRatesTracking;
    const Boards parameters;
    const Boards histograms;
    const double mainSleep;
    const bool readIntervalChange;
    const Boards resetCounters;
    const bool cleanupResetErrors;
    const std::optional<std::string> cleanupConfiguration;

    static Result<TesterConfig> fromToml(const toml::table& toml);
};

} // namespace tests
