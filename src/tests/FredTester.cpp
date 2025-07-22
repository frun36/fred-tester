#include "tests/FredTester.h"

#include <algorithm>
#include <thread>

#include "CommandTest.h"
#include "Logger.h"
#include "MapiHandler.h"
#include "dis.hxx"
#include "tests/Configurations.h"
#include "tests/HistogramsSingle.h"
#include "tests/HistogramsTracking.h"
#include "tests/ResetErrors.h"
#include "tests/ResetSystem.h"
#include "utils.h"

using namespace std::chrono;

namespace tests {

FredTester::FredTester(TesterConfig cfg, DimService* badChannelMap) :
    m_cfg(cfg),
    m_badChannelMap(badChannelMap) {
    for (auto board : cfg.statusTracking) {
        status.emplace_back(board, Status(board, cfg.connectedBoards));
    }

    for (auto board : cfg.counterRatesTracking) {
        counterRates.emplace_back(board, CounterRates(board));
    }
}

bool FredTester::setup() {
    bool res;

    if (m_cfg.resetSystem) {
        res = ResetSystem().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    if (m_cfg.managerStart) {
        Logger::info("MANAGER", "Sending START command");
        MapiHandler::sendCommand(utils::topic(utils::TCM0, "MANAGER"), "START");
        std::this_thread::sleep_for(1s);
    }

    if (m_cfg.setupResetErrors) {
        res = ResetErrors().runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(1s);
    }

    for (auto& s : status) {
        s.second.start();
    }

    if (m_cfg.setupConfiguration) {
        res = Configurations(*m_cfg.setupConfiguration).runAndLog();
        if (!res) {
            return false;
        }
        std::this_thread::sleep_for(2.5s);
    }

    if (m_cfg.waitForAttenuator) {
        Logger::info(
            "FRED_TESTER",
            "Waiting for the attenuator to stop being busy"
        );
        bool attenuatorBusy = true;
        auto attenuatorTest =
            TestBuilder("WAIT FOR ATTENUATOR")
                .mapiName(utils::topic(utils::TCM0, "PARAMETERS"))
                .command("ATTENUATOR_BUSY,READ")
                .pattern(R"(ATTENUATOR_BUSY,({})\n)", utils::FLT)
                .withValueValidator(
                    [&attenuatorBusy](auto match) -> Result<void> {
                        double res;
                        TRY_ASSIGN(utils::parseDouble(match[1]), res);
                        attenuatorBusy = (res != 0.);
                        return {};
                    }
                )
                .timeout(0.2)
                .expectOk()
                .build();

        while (attenuatorBusy) {
            if (!attenuatorTest.run())
                return false;
            std::this_thread::sleep_for(500ms);
        }
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
    if (m_cfg.cleanupConfiguration) {
        Configurations(*m_cfg.cleanupConfiguration).runAndLog();
        std::this_thread::sleep_for(2.5s);
    }

    if (m_cfg.cleanupResetErrors) {
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

    for (auto board : m_cfg.parameters) {
        Parameters(board).run();
        std::this_thread::sleep_for(2s);
    }

    for (auto board : m_cfg.histograms) {
        if (board.isTcm())
            tcmHistograms();
        else
            pmHistograms(board);
    }

    std::this_thread::sleep_for(std::chrono::duration<double>(m_cfg.mainSleep));
    for (auto& c : counterRates) {
        c.second.stop();
    }
    std::this_thread::sleep_for(10ms);
    if (m_cfg.badChannelMap) {
        std::string badChannelMap;
        for (auto& c : counterRates) {
            if (c.first.isTcm())
                continue;
            std::string currMap =
                c.second.getBadChannelMap(*m_cfg.badChannelMap);
            badChannelMap += currMap;
            currMap.pop_back();
            Logger::info(
                "BAD_CHANNEL_MAP",
                "{} bad channel map\n{}",
                c.first.name(),
                currMap
            );
        }
        publishBadChannelMap(badChannelMap);
    }

    if (m_cfg.readIntervalChange) {
        changeReadInterval();
        resetReadInterval();
    }

    std::this_thread::sleep_for(2s);

    for (auto board : m_cfg.resetCounters) {
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

void FredTester::publishBadChannelMap(std::string map) {
    if (m_badChannelMap != nullptr)
        m_badChannelMap->updateService((char*)map.c_str());
}

} // namespace tests
