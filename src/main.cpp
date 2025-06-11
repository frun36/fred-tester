#include <thread>

#include "tests/Configurations.h"
#include "tests/CounterRates.h"
#include "tests/Parameters.h"
#include "tests/Status.h"

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
        Logger::setDebug(true);
    }

    tests::Configurations configurations("laser_1124hz_or_trg");
    tests::Status tcmStatus("TCM0");
    tests::Status pmStatus("PMA0");
    tests::CounterRates tcmCounterRates("TCM0");
    tests::CounterRates pmCounterRates("PMA0");
    tests::Parameters tcmParameters("TCM0");
    tests::Parameters pmParameters("PMA0");

    configurations.run();

    tcmStatus.start();
    pmStatus.start();

    std::this_thread::sleep_for(std::chrono::duration<double>(2.));

    tcmCounterRates.start();
    pmCounterRates.start();

    std::this_thread::sleep_for(std::chrono::duration<double>(15.));

    tcmParameters.run();
    std::this_thread::sleep_for(std::chrono::duration<double>(1.));
    pmParameters.run();

    std::this_thread::sleep_for(std::chrono::duration<double>(15.));

    tcmStatus.stop();
    pmStatus.stop();

    tcmCounterRates.stop();
    tcmCounterRates.logSummary();

    pmCounterRates.stop();
    pmCounterRates.logSummary();
}
