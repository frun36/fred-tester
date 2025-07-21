#include "tests/FredTester.h"

#include <algorithm>
#include <thread>

#include "Logger.h"
#include "MapiHandler.h"
#include "tests/Configurations.h"
#include "tests/HistogramsSingle.h"
#include "tests/HistogramsTracking.h"
#include "tests/ResetErrors.h"
#include "tests/ResetSystem.h"
#include "utils.h"

using namespace std::chrono;

namespace tests {

FredTester::FredTester(TesterConfig cfg) : cfg(cfg) {
    for (auto board : cfg.statusTracking) {
        status.emplace_back(board, Status(board, cfg.connectedBoards));
    }

    for (auto board : cfg.counterRatesTracking) {
        counterRates.emplace_back(board, CounterRates(board));
    }
}

bool FredTester::setup() {
    bool res;

    if (cfg.resetSystem) {
        res = ResetSystem().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    if (cfg.managerStart) {
        Logger::info("MANAGER", "Sending START command");
        MapiHandler::sendCommand(utils::topic(utils::TCM0, "MANAGER"), "START");
        std::this_thread::sleep_for(1s);
    }

    if (cfg.setupResetErrors) {
        res = ResetErrors().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    for (auto& s : status) {
        s.second.start();
    }

    if (cfg.setupConfiguration) {
        res = Configurations(*cfg.setupConfiguration).runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(2.5s);
    }

    for (auto& c : counterRates) {
        c.second.start();
    }

    return true;
}

void FredTester::changeReadInterval() {
    Logger::info("COUNTER_RATES", "Read interval change");
    MapiHandler::sendCommand(
        topic(utils::TCM0, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,3"
    );
    for (auto& c : counterRates) {
        c.second.start(0.5 / 2.);
        c.second.setInterval(0.5 / 2.);
    }

    std::this_thread::sleep_for(5s);
    for (auto& c : counterRates) {
        c.second.stop(false);
    }
    std::this_thread::sleep_for(10ms);
}

void FredTester::resetReadInterval() {
    Logger::info("COUNTER_RATES", "Read interval change");
    MapiHandler::sendCommand(
        topic(utils::TCM0, "PARAMETERS"),
        "COUNTER_READ_INTERVAL,WRITE,4"
    );
    for (auto& c : counterRates) {
        c.second.start(1. / 2.);
        c.second.setInterval(1. / 2.);
    }
}

void FredTester::tcmHistograms() {
    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "STOP");
    std::this_thread::sleep_for(2s);

    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "COUNTER,0");
    std::this_thread::sleep_for(100ms);
    TcmHistogramsSingle(false).runAndLog();

    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "COUNTER,1");
    std::this_thread::sleep_for(100ms);
    TcmHistogramsSingle(true).runAndLog();

    TcmHistogramsTracking tcmHistogramsTracking {true};
    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "START");
    tcmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(utils::TCM0, "HISTOGRAMS"), "STOP");
    tcmHistogramsTracking.stop();
}

void FredTester::pmHistograms(utils::Board board) {
    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "STOP");
    std::this_thread::sleep_for(2s);

    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "SELECT");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(board, false, false, false).runAndLog();
    std::this_thread::sleep_for(100ms);

    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "SELECT,ADC0");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(board, true, false, false).runAndLog();

    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "SELECT,ADC1");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(board, false, true, false).runAndLog();

    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "SELECT,TIME");
    std::this_thread::sleep_for(100ms);
    PmHistogramsSingle(board, false, false, true).runAndLog();

    PmHistogramsTracking pmHistogramsTracking {board, false, false, true};
    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "START");
    pmHistogramsTracking.start();
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "RESET");
    std::this_thread::sleep_for(5s);
    MapiHandler::sendCommand(topic(board, "HISTOGRAMS"), "STOP");
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
    for (auto& s : status) {
        s.second.stop();
    }

    for (auto& c : counterRates) {
        c.second.stop(false);
    }
}

void FredTester::run() {
    if (!setup()) {
        finish();
        return;
    }

    std::this_thread::sleep_for(1s);

    for (auto board : cfg.parameters) {
        Parameters(board).run();
        std::this_thread::sleep_for(2s);
    }

    for (auto board : cfg.histograms) {
        if (board.isTcm())
            tcmHistograms();
        else
            pmHistograms(board);
    }

    std::this_thread::sleep_for(std::chrono::duration<double>(cfg.mainSleep));
    for (auto& c : counterRates) {
        c.second.stop();
    }
    std::this_thread::sleep_for(10ms);

    if (cfg.readIntervalChange) {
        changeReadInterval();
        resetReadInterval();
    }

    std::this_thread::sleep_for(2s);

    for (auto board : cfg.resetCounters) {
        auto it = std::find_if(
            counterRates.begin(),
            counterRates.end(),
            [board](const auto& p) { return p.first == board; }
        );
        if (it != counterRates.end()) {
            it->second.resetCounters();
            std::this_thread::sleep_for(5s);
        }
    }

    cleanup();

    finish();
}

} // namespace tests
