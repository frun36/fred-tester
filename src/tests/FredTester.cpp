#include "tests/FredTester.h"

#include <thread>

#include "MapiHandler.h"
#include "tests/Configurations.h"
#include "tests/HistogramsSingle.h"
#include "tests/HistogramsTracking.h"
#include "tests/ResetErrors.h"
#include "tests/ResetSystem.h"
#include "utils.h"

using namespace std::chrono;

namespace tests {

bool FredTester::setup() {
    bool res;

    if (cfg.resetSystem) {
        res = ResetSystem().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    if (cfg.setupResetErrors) {
        res = ResetErrors().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    Logger::info("MANAGER", "Sending START command");
    MapiHandler::sendCommand(utils::topic(utils::TCM, "MANAGER"), "START");
    std::this_thread::sleep_for(1s);

    tcmStatus.start();
    pmStatus.start();

    if (cfg.setupConfiguration) {
        res = Configurations(*cfg.setupConfiguration).runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(2.5s);
    }

    tcmCounterRates.start();
    pmCounterRates.start();

    return true;
}

void FredTester::changeReadInterval() {
    Logger::info("COUNTER_RATES", "Read interval change");
    MapiHandler::sendCommand(
        topic(TCM, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,3"
    );
    tcmCounterRates.start(0.5 / 2.); // 2x faster responses than read interval
    pmCounterRates.start(0.5 / 2.);

    std::this_thread::sleep_for(5s);
    tcmCounterRates.stop(false);
    pmCounterRates.stop(false);
    std::this_thread::sleep_for(10ms);
}

void FredTester::resetReadInterval() {
    Logger::info("COUNTER_RATES", "Read interval change");
    MapiHandler::sendCommand(
        topic(TCM, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,4"
    );
    tcmCounterRates.start(1. / 2.); // 2x faster responses than read interval
    pmCounterRates.start(1. / 2.);
}

void FredTester::tcmHistograms() {
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "STOP");
    std::this_thread::sleep_for(2s);

    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "COUNTER,0");
    std::this_thread::sleep_for(100ms);
    TcmHistogramsSingle(false).runAndLog();

    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "COUNTER,1");
    std::this_thread::sleep_for(100ms);
    TcmHistogramsSingle(true).runAndLog();

    TcmHistogramsTracking tcmHistogramsTracking {true};
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "START");
    tcmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM, "HISTOGRAMS"), "STOP");
    tcmHistogramsTracking.stop();
}

void FredTester::pmHistograms() {
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "STOP");
    std::this_thread::sleep_for(2s);

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, false, false).runAndLog();
    std::this_thread::sleep_for(100ms);

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,ADC0");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(true, false, false).runAndLog();

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,ADC1");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, true, false).runAndLog();

    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "SELECT,TIME");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(false, false, true).runAndLog();

    PmHistogramsTracking pmHistogramsTracking {false, false, true};
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "START");
    pmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::PM, "HISTOGRAMS"), "STOP");
    pmHistogramsTracking.stop();
}

void FredTester::cleanup() {
    if (cfg.cleanupConfiguration) {
        Configurations(*cfg.cleanupConfiguration).runAndLog();
        std::this_thread::sleep_for(2.5s);
    }

    if (cfg.cleanupResetErrors) {
        ResetErrors().runAndLog();
        std::this_thread::sleep_for(1s);
    }
}

void FredTester::finish() {
    tcmStatus.stop();
    pmStatus.stop();

    tcmCounterRates.stop(false);
    pmCounterRates.stop(false);
}

void FredTester::run() {
    if (!setup()) {
        return;
    }

    std::this_thread::sleep_for(1s);

    if (cfg.tcmParameters) {
        Parameters(utils::TCM).run();
        std::this_thread::sleep_for(2s);
    }
    if (!cfg.pmParameters.empty()) {
        Parameters(utils::PM).run();
        std::this_thread::sleep_for(2s);
    }

    if (cfg.tcmHistograms) {
        tcmHistograms();
    }
    if (!cfg.pmHistograms.empty()) {
        pmHistograms();
    }

    std::this_thread::sleep_for(std::chrono::duration<double>(cfg.mainSleep));
    tcmCounterRates.stop();
    pmCounterRates.stop();
    std::this_thread::sleep_for(10ms);

    if (cfg.readIntervalChange) {
        changeReadInterval();
        resetReadInterval();
    }

    std::this_thread::sleep_for(2s);

    if (cfg.tcmResetCounters) {
        tcmCounterRates.resetCounters();
        std::this_thread::sleep_for(5s);
    }

    if (!cfg.pmResetCounters.empty()) {
        pmCounterRates.resetCounters();
        std::this_thread::sleep_for(5s);
    }

    cleanup();

    finish();
}

} // namespace tests
