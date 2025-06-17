#include "tests/FredTester.h"

using namespace std::chrono;

namespace tests {

void FredTester::run() {
    configurations.run();

    tcmStatus.start();
    pmStatus.start();

    std::this_thread::sleep_for(2s);

    tcmCounterRates.start();
    pmCounterRates.start();

    std::this_thread::sleep_for(10s);

    tcmParameters.run();
    std::this_thread::sleep_for(1s);
    pmParameters.run();
    std::this_thread::sleep_for(5s);

    tcmCounterRates.stop();
    pmCounterRates.stop();
    std::this_thread::sleep_for(10ms);
    MapiHandler::sendCommand(
        topic(TCM, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,3"
    );
    tcmCounterRates.start(0.5);
    pmCounterRates.start(0.5);

    std::this_thread::sleep_for(5s);

    tcmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    pmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    tcmStatus.stop();
    pmStatus.stop();

    tcmCounterRates.stop();
    pmCounterRates.stop();
}

} // namespace tests
