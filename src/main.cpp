#include <thread>

#include "tests/Configurations.h"
#include "tests/Parameters.h"
#include "tests/Status.h"
#include "tests/TcmCounterRates.h"

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
        Logger::setDebug(true);
    }

    tests::Configurations configurations("fitlab_laser_1122hz");
    tests::Status status;
    tests::TcmCounterRates tcmCounterRates;
    tests::TcmParameters parameters;

    configurations.run();

    status.start();

    std::this_thread::sleep_for(std::chrono::duration<double>(2.));

    tcmCounterRates.start();

    std::this_thread::sleep_for(std::chrono::duration<double>(3.));

    parameters.run();

    std::this_thread::sleep_for(std::chrono::duration<double>(3.));

    status.stop();
    tcmCounterRates.stop();
    tcmCounterRates.logSummary();
}
