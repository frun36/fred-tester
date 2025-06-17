#include "tests/FredTester.h"

using namespace std::chrono;

namespace tests {

bool FredTester::setup() {
    bool res;

    res = resetSystem.runAndLog();
    if (!res) {
        return false;
    }

    tcmStatus.start();
    pmStatus.start();

    res = configurations.runAndLog();
    if (!res) {
        return false;
    }

    std::this_thread::sleep_for(2s);

    tcmCounterRates.start();
    pmCounterRates.start();

    return true;
}

void FredTester::changeReadInterval() {
    tcmCounterRates.stop();
    pmCounterRates.stop();
    std::this_thread::sleep_for(10ms);
    MapiHandler::sendCommand(
        topic(TCM, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,3"
    );
    tcmCounterRates.start(0.5);
    pmCounterRates.start(0.5);
}

void FredTester::finish() {
    tcmStatus.stop();
    pmStatus.stop();

    tcmCounterRates.stop();
    pmCounterRates.stop();
}

void FredTester::run() {
    if (!setup()) {
        return;
    }

    std::this_thread::sleep_for(10s);

    tcmParameters.run();
    std::this_thread::sleep_for(1s);
    pmParameters.run();
    std::this_thread::sleep_for(5s);

    changeReadInterval();

    std::this_thread::sleep_for(5s);

    tcmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    pmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    finish();
}

} // namespace tests
