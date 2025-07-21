#include <argparse/argparse.hpp>
#include <cstring>
#include <string>
#include <toml++/toml.hpp>

#include "FredTesterRpc.h"
#include "Logger.h"
#include "tests/FredTester.h"
#include "tests/TesterConfig.h"

struct Cli {
    const bool debug;
    const std::string mode;
    const std::string configFile;

    Cli(const argparse::ArgumentParser& program) :
        debug(program.get<bool>("--debug")),
        mode(program.get("mode")),
        configFile(program.get("--config-file")) {}

    static Result<Cli> parseArguments(int argc, char** argv) {
        argparse::ArgumentParser program("fred-tester");

        program.add_argument("-d", "--debug")
            .help("Enable debug mode")
            .default_value(false)
            .implicit_value(true);

        program.add_argument("mode")
            .help(
                "Operation mode: 'single' (single pass of the tester) or 'dim' (run a DIM server accepting commands)"
            )
            .choices("single", "dim");

        program.add_argument("-c", "--config-file")
            .help("Path to the config file (in run mode)")
            .default_value("../default.toml");

        try {
            program.parse_args(argc, argv);
        } catch (const std::runtime_error& e) {
            std::cerr << program;
            return err("Failed to parse program arguments: {}", e.what());
        }

        return Cli(program);
    }
};

Result<void> runSingle(std::string configFile) {
    toml::parse_result toml = toml::parse_file(configFile);
    if (!toml) {
        return err("TOML parsing error: {}", toml.error().description());
    }

    auto cfgRes = tests::TesterConfig::fromToml(toml.table());

    if (!cfgRes) {
        return err("Failed to construct config: {}", cfgRes.error());
    }

    auto cfg = *cfgRes;

    Logger::info("FRED_TESTER", "Running single test from {}", configFile);
    tests::FredTester tester(cfg);
    tester.run();
    return {};
}

Result<void> runDim() {
    FredTesterRpc rpc;

    DimService dim("FRED_TESTER/LOG", (char*)"");
    Logger::initDim(&dim);
    DimServer::start("FRED_TESTER");

    Logger::info("FRED_TESTER", "Started DIM server");
    while (true) {
        auto _ = rpc.waitAndExecute();
    }
}

int main(int argc, char** argv) {
    auto cli = Cli::parseArguments(argc, argv);

    if (!cli) {
        Logger::error("FRED_TESTER", "{}", cli.error());
    }

    if (cli->debug) {
        Logger::setDebug(true);
        Logger::debug("FRED_TESTER", "Debug mode enabled");
    }

    Result<void> res;
    if (cli->mode == "dim") {
        res = runDim();
    } else if (cli->mode == "single") {
        res = runSingle(cli->configFile);
    } else {
        res = err("Unsupported mode: {}", cli->mode);
    }

    if (!res) {
        Logger::error("FRED_TESTER", "{}", res.error());
        return 1;
    } else {
        Logger::info("FRED_TESTER", "DONE");
        return 0;
    }
}
