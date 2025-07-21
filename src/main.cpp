#include <argparse/argparse.hpp>
#include <cstring>
#include <string>
#include <toml++/toml.hpp>

#include "Logger.h"
#include "tests/FredTester.h"
#include "tests/TesterConfig.h"

int main(int argc, char** argv) {
    argparse::ArgumentParser program("fred-tester");

    program.add_argument("-d", "--debug")
        .help("Enable debug mode")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("mode")
        .help(
            "Operation mode: 'run' (single pass of the tester) or 'dim' (run a DIM server accepting commands)"
        )
        .choices("run", "dim");

    program.add_argument("-c", "--config-file")
        .help("Path to the config file (in run mode)")
        .default_value("../default.toml");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        Logger::error(
            "FRED_TESTER",
            "Failed to parse program arguments: {}",
            err.what()
        );
        std::cerr << program;
        std::exit(1);
    }

    if (program.get<bool>("--debug")) {
        Logger::setDebug(true);
        Logger::debug("FRED_TESTER", "Debug mode enabled");
    }

    if (program.get<std::string>("mode") == "dim") {
        std::cout << "Running as DIM server";
    } else {
        toml::parse_result toml =
            toml::parse_file(program.get<std::string>("--config-file"));
        if (!toml) {
            Logger::error(
                "FRED_TESTER",
                "TOML parsing error: {}",
                toml.error().description()
            );
            exit(1);
        }

        auto cfgRes = tests::TesterConfig::fromToml(toml.table());

        if (!cfgRes) {
            Logger::error(
                "FRED_TESTER",
                "Failed to process config: {}",
                cfgRes.error()
            );
            exit(1);
        }

        auto cfg = *cfgRes;

        tests::FredTester tester(cfg);
        tester.run();
    }

    return 0;
}
