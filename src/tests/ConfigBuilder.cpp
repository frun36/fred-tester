#include "tests/ConfigBuilder.h"

#include "Logger.h"

namespace tests {

Result<TesterConfig> ConfigBuilder::parseToml(const toml::table& t) {
    auto setupOpt = t.get_as<toml::table>("setup");
    auto testsOpt = t.get_as<toml::table>("tests");
    auto cleanupOpt = t.get_as<toml::table>("cleanup");

    if (!setupOpt || !testsOpt || !cleanupOpt) {
        return err("Config file must contain [setup], [tests], and [cleanup]");
    }

    TRY(handleTable(*setupOpt, setupOptions));
    TRY(handleTable(*testsOpt, testsOptions));
    TRY(handleTable(*cleanupOpt, cleanupOptions));

    return TesterConfig {
        *setupTryGet<TesterConfig::Boards>("connected_boards"),
        *setupTryGet<bool>("reset_system"),
        *setupTryGet<bool>("manager_start"),
        *setupTryGet<bool>("reset_errors"),
        *setupTryGet<TesterConfig::Boards>("status_tracking"),
        *setupTryGet<std::optional<std::string>>("sc_status_tracking"),
        *setupTryGet<std::optional<std::string>>("configuration"),
        *setupTryGet<bool>("wait_for_attenuator"),
        *setupTryGet<TesterConfig::Boards>("counter_rates_tracking"),
        *setupTryGet<std::optional<TesterConfig::BadChannelMapConfig>>(
            "bad_channel_map"
        ),
        *testTryGet<TesterConfig::Boards>("parameters"),
        *testTryGet<bool>("test_parameters"),
        *testTryGet<bool>("reset"),
        *testTryGet<TesterConfig::Boards>("histograms"),
        *testTryGet<double>("main_sleep"),
        *testTryGet<bool>("read_interval_change"),
        *testTryGet<TesterConfig::Boards>("reset_counters"),
        *cleanupTryGet<bool>("reset_errors"),
        *cleanupTryGet<std::optional<std::string>>("configuration"),
    };
}

Result<void> ConfigBuilder::handleTable(const toml::table& t, Options& o) {
    for (auto& [key, val] : o) {
        switch (val->type) {
            case BaseEntry::Type::Bool: {
                auto val = parse<bool>(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug("CONFIG", "Boolean {} = {}", key, *val);
                setupSet(key, *val);
                break;
            }
            case BaseEntry::Type::String: {
                auto val = parse<std::string>(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug("CONFIG", "String {} = {}", key, *val);
                setupSet(key, *val);
                break;
            }
            case BaseEntry::Type::Double: {
                auto val = parse<double>(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug("CONFIG", "Double {} = {}", key, *val);
                setupSet(key, *val);
                break;
            }
            case BaseEntry::Type::OptionalString: {
                auto val = parseOptional<std::string>(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug(
                    "CONFIG",
                    "Optional {} = {}",
                    key,
                    val->value_or("<none>")
                );
                setupSet(key, *val);
                break;
            }
            case BaseEntry::Type::Boards: {
                auto val = parseBoards(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug(
                    "CONFIG",
                    "Boards {} size {}",
                    key,
                    val->size()
                );
                setupSet(key, *val);
                break;
            }
            case BaseEntry::Type::BadChannelMap: {
                auto val = parseBadChannelMap(t, key);
                if (!val) {
                    return std::unexpected(val.error());
                }
                Logger::debug(
                    "CONFIG",
                    "BadChannelMap {} enabled = {}",
                    key,
                    val->has_value()
                );
                setupSet(key, *val);
                break;
            }
            default:
                return err("Unexpected type for '{}'", key);
        }
    }

    return {};
}

Result<std::optional<TesterConfig::BadChannelMapConfig>> ConfigBuilder::
    parseBadChannelMap(const toml::table& t, std::string name) {
    auto* node = t.get(name);

    if (!node)
        return std::nullopt;

    if (node->is_boolean()) {
        if (auto b = node->value<bool>(); b && !*b)
            return std::nullopt;
    }

    if (node->is_table()) {
        auto result =
            TesterConfig::BadChannelMapConfig::fromToml(*node->as_table());
        if (!result)
            return std::unexpected(result.error());
        return *result;
    }

    return err("{} must be a table or false", name);
}

Result<TesterConfig::Boards> ConfigBuilder::parseBoards(
    const toml::table& t,
    std::string name
) {
    if (name == "connected_boards") {
        TesterConfig::Boards connectedBoards;
        if (auto array = t.get_as<toml::array>(name)) {
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
        return connectedBoards;
    }

    auto* node = t.get(name);
    if (!node)
        return err("{} missing", name);
    auto connectedBoards =
        setupGetOr<TesterConfig::Boards>("connected_boards", {});
    if (node->is_boolean()) {
        return *node->value<bool>() ? connectedBoards : TesterConfig::Boards {};
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
    return err("{} must be a boolean or array of strings", std::string(name));
};

}; // namespace tests
