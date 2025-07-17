#pragma once

#include <optional>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

#include "Result.h"

struct TesterConfig {
    using Boards = std::vector<std::string>;

    const bool resetSystem;
    const bool setupResetErrors;
    const Boards statusTracking;
    const std::optional<std::string> setupConfiguration;
    const bool tcmParameters;
    const Boards pmParameters;
    const bool tcmHistograms;
    const Boards pmHistograms;
    const double mainSleep;
    const bool readIntervalChange;
    const bool tcmResetCounters;
    const Boards pmResetCounters;
    const bool cleanupResetErrors;
    const std::optional<std::string> cleanupConfiguration;

    static Result<TesterConfig> fromToml(const toml::table& toml) {
        auto setupOpt = toml.get_as<toml::table>("setup");
        auto testsOpt = toml.get_as<toml::table>("tests");
        auto cleanupOpt = toml.get_as<toml::table>("cleanup");

        if (!setupOpt || !testsOpt || !cleanupOpt) {
            return err(
                "Config file must contain [setup], [tests], and [cleanup]"
            );
        }

        auto setup = *setupOpt;
        auto tests = *testsOpt;
        auto cleanup = *cleanupOpt;

        // -- connected_boards
        Boards connectedBoards;
        if (auto array = setup.get_as<toml::array>("connected_boards")) {
            for (const auto& node : *array) {
                if (auto val = node.value<std::string>()) {
                    connectedBoards.push_back(*val);
                } else {
                    return err("connected_boards must contain only strings");
                }
            }
        } else {
            return err("[setup].connected_boards must be an array of strings");
        }

        // -- Helpers

        auto parseBoards = [&](toml::node* node,
                               std::string_view name) -> Result<Boards> {
            if (!node)
                return err("{} missing", std::string(name));
            if (node->is_boolean()) {
                return *node->value<bool>() ? connectedBoards : Boards {};
            } else if (node->is_array()) {
                Boards boards;
                auto arr = node->as_array();
                if (!arr)
                    return err("{} must be an array", std::string(name));
                for (const auto& n : *arr) {
                    if (auto val = n.value<std::string>()) {
                        boards.push_back(*val);
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
                return std::optional<
                    std::string> {}; // treat missing as nullopt
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

        auto tcmParameters = parseBool(tests, "tcm_parameters");
        if (!tcmParameters)
            return std::unexpected(tcmParameters.error());

        auto pmParameters =
            parseBoards(tests.get("pm_parameters"), "pm_parameters");
        if (!pmParameters)
            return std::unexpected(pmParameters.error());

        auto readIntervalChange = parseBool(tests, "read_interval_change");
        if (!readIntervalChange)
            return std::unexpected(readIntervalChange.error());

        auto tcmHistograms = parseBool(tests, "tcm_histograms");
        if (!tcmHistograms)
            return std::unexpected(tcmHistograms.error());

        auto pmHistograms =
            parseBoards(tests.get("pm_histograms"), "pm_histograms");
        if (!pmHistograms)
            return std::unexpected(pmHistograms.error());

        auto mainSleep = parseDouble(tests, "main_sleep");
        if (!mainSleep)
            return std::unexpected(mainSleep.error());

        auto tcmResetCounters = parseBool(tests, "tcm_reset_counters");
        if (!tcmResetCounters)
            return std::unexpected(tcmResetCounters.error());

        auto pmResetCounters =
            parseBoards(tests.get("pm_reset_counters"), "pm_reset_counters");
        if (!pmResetCounters)
            return std::unexpected(pmResetCounters.error());

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
            *resetSystem,
            *setupResetErrors,
            *statusTracking,
            *setupConfiguration,
            *tcmParameters,
            *pmParameters,
            *tcmHistograms,
            *pmHistograms,
            *mainSleep,
            *readIntervalChange,
            *tcmResetCounters,
            *pmResetCounters,
            *cleanupResetErrors,
            *cleanupConfiguration,
        };
    }
};
