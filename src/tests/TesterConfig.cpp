#include "tests/TesterConfig.h"

#include "Result.h"
#include "tests/ConfigBuilder.h"

namespace tests {

Result<TesterConfig> TesterConfig::fromToml(const toml::table& t) {
    return ConfigBuilder()
        .setup<Boards>("connected_boards")
        .setup<bool>("reset_system")
        .setup<bool>("manager_start")
        .setup<bool>("reset_errors")
        .setup<Boards>("status_tracking")
        .setup<std::optional<std::string>>("sc_status_tracking")
        .setup<std::optional<std::string>>("configuration")
        .setup<bool>("wait_for_attenuator")
        .setup<Boards>("counter_rates_tracking")
        .setup<std::optional<BadChannelMapConfig>>("bad_channel_map")
        .test<Boards>("parameters") 
        .test<bool>("test_parameters")
        .test<Boards>("histograms")
        .test<double>("main_sleep")
        .test<bool>("read_interval_change")
        .test<Boards>("reset_counters")
        .cleanup<bool>("reset_errors")
        .cleanup<std::optional<std::string>>("configuration")
        .parseToml(t);
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
        && rate <= (1. + upperTolerance) * expectedRate;
}
} // namespace tests
