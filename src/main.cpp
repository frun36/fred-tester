#include <cstring>
#include <string>

#include "Logger.h"
#include "argparse/argparse.hpp"
#include "tests/FredTester.h"

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

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        Logger::error(
            "FRED_TESTER",
            "Failed to parse program arguments: {}",
            err.what()
        );
        std::cout << program;
        std::exit(1);
    }

    if (program.get<bool>("--debug")) {
        Logger::setDebug(true);
        Logger::debug("FRED_TESTER", "Debug mode enabled");
    }

    if (program.get<std::string>("mode") == "dim") {
        std::cout << "Running as DIM server";
    } else {
        std::cout << "Single run";
        // tests::FredTester tester;
        // tester.run();
    }
}
