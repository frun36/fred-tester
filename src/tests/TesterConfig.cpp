#include "tests/TesterConfig.h"

#include "Result.h"

namespace tests {

Result<TesterConfig> TesterConfig::fromToml(const toml::table& t) {
    auto setupOpt = t.get_as<toml::table>("setup");
    auto testsOpt = t.get_as<toml::table>("tests");
    auto cleanupOpt = t.get_as<toml::table>("cleanup");

    if (!setupOpt || !testsOpt || !cleanupOpt) {
        return err("Config file must contain [setup], [tests], and [cleanup]");
    }

    auto setup = *setupOpt;
    auto tests = *testsOpt;
    auto cleanup = *cleanupOpt;

    // -- connected_boards
    TesterConfig::Boards connectedBoards;
    if (auto array = setup.get_as<toml::array>("connected_boards")) {
        for (const auto& node : *array) {
            if (auto val = node.value<std::string>()) {
                utils::Board board;
                TRY_ASSIGN(utils::Board::fromName(*val), board);
                connectedBoards.push_back(board);
            } else {
                return err("connected_boards must contain only strings");
            }
        }
    } else {
        return err("[setup].connected_boards must be an array of strings");
    }

    // -- Helpers

    auto parseBoards = [connectedBoards](
                           toml::node* node,
                           std::string_view name
                       ) -> Result<TesterConfig::Boards> {
        if (!node)
            return err("{} missing", std::string(name));
        if (node->is_boolean()) {
            return *node->value<bool>() ? connectedBoards
                                        : TesterConfig::Boards {};
        } else if (node->is_array()) {
            TesterConfig::Boards boards;
            auto arr = node->as_array();
            if (!arr)
                return err("{} must be an array", std::string(name));
            for (const auto& n : *arr) {
                if (auto val = n.value<std::string>()) {
                    utils::Board board;
                    TRY_ASSIGN(utils::Board::fromName(*val), board);
                    if (!std::ranges::contains(connectedBoards, board)) {
                        return err(
                            "Board {} is not declared as connected",
                            board.name()
                        );
                    }
                    boards.push_back(board);
                } else {
                    return err(
                        "{} array must contain only strings",
                        std::string(name)
                    );
                }
            }
            return boards;
        }
        return err(
            "{} must be a boolean or array of strings",
            std::string(name)
        );
    };

    auto parseOptionalString = [](toml::node* node, std::string_view name
                               ) -> Result<std::optional<std::string>> {
        if (!node)
            return std::optional<std::string> {};
        if (node->is_boolean()) {
            if (auto b = node->value<bool>(); b && !*b)
                return std::optional<std::string> {};
        } else if (auto str = node->value<std::string>()) {
            return *str;
        } else {
            return err("{} must be a string or false", std::string(name));
        }
        return std::optional<std::string> {};
    };

    auto parseBool = [](const toml::table& tbl,
                        std::string_view key) -> Result<bool> {
        auto val = tbl.get_as<bool>(key);
        if (!val)
            return err("{} must be a boolean", std::string(key));
        return **val;
    };

    auto parseDouble = [](const toml::table& tbl,
                          std::string_view key) -> Result<double> {
        auto val = tbl.get_as<double>(key);
        if (!val)
            return err("{} must be a double", std::string(key));
        return **val;
    };

    // -- Parse each field with error handling

    auto resetSystem = parseBool(setup, "reset_system");
    if (!resetSystem)
        return std::unexpected(resetSystem.error());

    auto managerStart = parseBool(setup, "manager_start");
    if (!managerStart)
        return std::unexpected(managerStart.error());

    auto setupResetErrors = parseBool(setup, "reset_errors");
    if (!setupResetErrors)
        return std::unexpected(setupResetErrors.error());

    auto statusTracking =
        parseBoards(setup.get("status_tracking"), "status_tracking");
    if (!statusTracking)
        return std::unexpected(statusTracking.error());

    auto setupConfiguration =
        parseOptionalString(setup.get("configuration"), "configuration");
    if (!setupConfiguration)
        return std::unexpected(setupConfiguration.error());

    auto waitForAttenuator = parseBool(setup, "wait_for_attenuator");
    if (!waitForAttenuator)
        return std::unexpected(waitForAttenuator.error());

    auto counterRatesTracking = parseBoards(
        setup.get("counter_rates_tracking"),
        "counter_rates_tracking"
    );
    if (!counterRatesTracking)
        return std::unexpected(counterRatesTracking.error());

    auto badChannelMapToml = setup.get("bad_channel_map");
    std::optional<BadChannelMapConfig> badChannelMap = std::nullopt;
    if (badChannelMapToml && badChannelMapToml->is_table()) {
        TRY_ASSIGN(
            BadChannelMapConfig::fromToml(*badChannelMapToml->as_table()),
            badChannelMap
        );
    }

    auto parameters = parseBoards(tests.get("parameters"), "parameters");
    if (!parameters)
        return std::unexpected(parameters.error());

    auto readIntervalChange = parseBool(tests, "read_interval_change");
    if (!readIntervalChange)
        return std::unexpected(readIntervalChange.error());

    auto histograms = parseBoards(tests.get("histograms"), "histograms");
    if (!histograms)
        return std::unexpected(histograms.error());

    auto mainSleep = parseDouble(tests, "main_sleep");
    if (!mainSleep)
        return std::unexpected(mainSleep.error());

    auto resetCounters =
        parseBoards(tests.get("reset_counters"), "reset_counters");
    if (!resetCounters)
        return std::unexpected(resetCounters.error());

    auto cleanupResetErrors = parseBool(cleanup, "reset_errors");
    if (!cleanupResetErrors)
        return std::unexpected(cleanupResetErrors.error());

    auto cleanupConfiguration = parseOptionalString(
        cleanup.get("configuration"),
        "cleanup.configuration"
    );

    if (!cleanupConfiguration)
        return std::unexpected(cleanupConfiguration.error());

    // -- Construct final object

    return TesterConfig {
        connectedBoards,
        *resetSystem,
        *managerStart,
        *setupResetErrors,
        *statusTracking,
        *setupConfiguration,
        *waitForAttenuator,
        *counterRatesTracking,
        badChannelMap,
        *parameters,
        *histograms,
        *mainSleep,
        *readIntervalChange,
        *resetCounters,
        *cleanupResetErrors,
        *cleanupConfiguration,
    };
}

Result<TesterConfig::BadChannelMapConfig> TesterConfig::BadChannelMapConfig::
    fromToml(const toml::table& toml) {
    auto parseDouble = [](const toml::table& tbl,
                          std::string_view key) -> Result<double> {
        auto val = tbl.get_as<double>(key);
        if (!val)
            return err("{} must be a double", std::string(key));
        return **val;
    };

    double expectedRate;
    TRY_ASSIGN(parseDouble(toml, "expected_rate"), expectedRate);
    double lowerTolerance;
    TRY_ASSIGN(parseDouble(toml, "lower_tolerance"), lowerTolerance);
    double upperTolerance;
    TRY_ASSIGN(parseDouble(toml, "upper_tolerance"), upperTolerance);
    auto referenceChannelName = toml.get_as<std::string>("reference_channel");
    if (!referenceChannelName->is_string()) {
        return err("Reference channel name must be a string");
    }
    utils::Channel referenceChannel;
    TRY_ASSIGN(
        utils::Channel::fromStr(referenceChannelName->as_string()->get()),
        referenceChannel
    );

    return BadChannelMapConfig {
        expectedRate,
        lowerTolerance,
        upperTolerance,
        referenceChannel
    };
}

bool TesterConfig::BadChannelMapConfig::validateValue(
    utils::Channel channel,
    double rate
) {
    if (channel == referenceChannel) {
        expectedRate *= 2;
    }

    return (1. - lowerTolerance) * expectedRate <= rate
        && rate <= (1. + upperTolerance);
}
} // namespace tests
