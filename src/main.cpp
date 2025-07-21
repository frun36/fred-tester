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
    const bool isSingle;
    const std::string configFile;

    Cli(argparse::ArgumentParser&& program) :
        debug(program.get<bool>("--debug")),
        isSingle(program.is_subcommand_used("single")),
        configFile(
            program.is_subcommand_used("single")
                ? program.at<argparse::ArgumentParser>("single")
                      .get<std::string>("config-file")
                : ""
        ) {}

    static Result<Cli> parseArguments(int argc, char** argv) {
        argparse::ArgumentParser program("fred-tester");

        program.add_argument("-d", "--debug")
            .help("Enable debug mode")
            .default_value(false)
            .implicit_value(true);

        argparse::ArgumentParser singleCmd("single");
        argparse::ArgumentParser dimCmd("dim");

        // program.add_argument("mode")
        //     .help(
        //         "Operation mode: 'single' (single pass of the tester) or 'dim' (run a DIM server accepting commands)"
        //     )
        //     .choices("single", "dim");

        singleCmd.add_argument("config-file")
            .help("Path to the config file (in 'single' mode)")
            .default_value("../default.toml");

        program.add_subparser(singleCmd);
        program.add_subparser(dimCmd);

        try {
            program.parse_args(argc, argv);
        } catch (const std::runtime_error& e) {
            std::cerr << program;
            return err("Failed to parse program arguments: {}", e.what());
        }

        return Cli(std::move(program));
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

    Logger::initDim();
    DimServer::start("FRED_TESTER");

    Logger::info("FRED_TESTER", "Started DIM server");
    while (true) {
        Logger::info("FRED_TESTER", "Waiting for DIM command");
        rpc.waitAndExecute();
        Logger::info("FRED_TESTER", "DONE");
    }
}

int main(int argc, char** argv) {
    auto cli = Cli::parseArguments(argc, argv);

    if (!cli) {
        Logger::error("FRED_TESTER", "{}", cli.error());
        return 1;
    }

    if (cli->debug) {
        Logger::setDebug(true);
        Logger::debug("FRED_TESTER", "Debug mode enabled");
    }

    Result<void> res = cli->isSingle ? runSingle(cli->configFile) : runDim();

    if (!res) {
        Logger::error("FRED_TESTER", "{}", res.error());
        return 1;
    } else {
        Logger::info("FRED_TESTER", "DONE");
        return 0;
    }
}
