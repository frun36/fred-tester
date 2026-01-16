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

    struct BadChannelMapConfig {
        double expectedRate;
        double lowerTolerance;
        double upperTolerance;
        utils::Channel referenceChannel;

        static Result<BadChannelMapConfig> fromToml(const toml::table& toml);

        bool validateValue(utils::Channel channel, double rate);
    };

    const Boards connectedBoards;
    const bool resetSystem;
    const bool managerStart;
    const bool setupResetErrors;
    const Boards statusTracking;
    const std::optional<std::string> scStatusTracking;
    const std::optional<std::string> setupConfiguration;
    const bool waitForAttenuator;
    const Boards counterRatesTracking;
    const std::optional<BadChannelMapConfig> badChannelMap;
    const Boards parameters;
    const bool testParameters;
    const bool reset;
    const Boards histograms;
    const double mainSleep;
    const bool readIntervalChange;
    const Boards resetCounters;
    const bool cleanupResetErrors;
    const std::optional<std::string> cleanupConfiguration;

    static Result<TesterConfig> fromToml(const toml::table& toml);
};

} // namespace tests
