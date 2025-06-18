#include "tests/FredTester.h"

#include <thread>

#include "MapiHandler.h"
#include "tests/HistogramsSingle.h"
#include "tests/HistogramsTracking.h"
#include "utils.h"

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

void FredTester::histograms() {
    TcmHistogramsSingle(false).runAndLog();
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, false, false).runAndLog();
    std::this_thread::sleep_for(100ms);

    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "COUNTER,1");
    std::this_thread::sleep_for(100ms);
    TcmHistogramsSingle(true).runAndLog();

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,ADC0");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(true, false, false).runAndLog();

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,ADC1");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, true, false).runAndLog();

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,TIME");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, false, true).runAndLog();

    TcmHistogramsTracking tcmHistogramsTracking {true};
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "START");
    tcmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "STOP");
    tcmHistogramsTracking.stop();

    PmHistogramsTracking pmHistogramsTracking {false, false, true};
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "START");
    pmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "STOP");
    pmHistogramsTracking.stop();
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

    Parameters(utils::TCM).run();
    std::this_thread::sleep_for(1s);
    Parameters(utils::PM).run();
    std::this_thread::sleep_for(5s);

    changeReadInterval();

    std::this_thread::sleep_for(1s);

    histograms();

    std::this_thread::sleep_for(1s);

    tcmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    pmCounterRates.resetCounters();
    std::this_thread::sleep_for(5s);

    finish();
}

} // namespace tests
