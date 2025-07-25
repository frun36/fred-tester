#include "tests/CounterRates.h"

#include <format>
#include <ranges>
#include <utility>

#include "Logger.h"
#include "Result.h"
#include "utils.h"

using namespace utils;

namespace tests {

CounterRates::Response::Response(
    std::string readIntervalState,
    double readIntervalSeconds,
    std::string fifoState,
    uint32_t fifoLoad,
    std::string fifoReadResult,
    std::vector<uint32_t> counters,
    std::vector<double> rates,
    double prevElapsed
) :
    readIntervalState(std::move(readIntervalState)),
    readIntervalSeconds(readIntervalSeconds),
    fifoState(std::move(fifoState)),
    fifoLoad(fifoLoad),
    fifoReadResult(std::move(fifoReadResult)),
    counters(std::move(counters)),
    rates(std::move(rates)),
    prevElapsed(prevElapsed) {}

Result<CounterRates::Response> CounterRates::Response::fromMatch(
    std::string testName,
    std::smatch match,
    size_t numberOfCounters
) {
    if (match.size() < 37) {
        return err("Match contains too few matched groups: {}", match.size());
    }

    auto it = match.begin() + 1;

    std::string readIntervalState = *it++;

    double readIntervalSeconds;
    TRY_ASSIGN(parseDouble(*it++), readIntervalSeconds);

    std::string fifoState = *it++;

    uint32_t fifoLoad;
    TRY_ASSIGN(parseInt(*it++), fifoLoad);

    std::string fifoReadResult = *it++;

    std::vector<uint32_t> counters(numberOfCounters);
    if (!it->matched) {
        Logger::warn(testName, "No counters");
        counters.clear();
        it += numberOfCounters;
    } else {
        for (size_t i = 0; i < numberOfCounters; i++) {
            TRY_ASSIGN(parseInt(*it++), counters[i]);
        }
    }

    std::vector<double> rates(numberOfCounters);
    if (!it->matched) {
        Logger::warn(testName, "No rates");
        rates.clear();
        it += numberOfCounters;
    } else {
        for (size_t i = 0; i < numberOfCounters; i++) {
            TRY_ASSIGN(parseDouble(*it++), rates[i]);
        }
    }

    double prevElapsed;
    TRY_ASSIGN(parseDouble(*it++), prevElapsed);

    return Response(
        std::move(readIntervalState),
        readIntervalSeconds,
        std::move(fifoState),
        fifoLoad,
        std::move(fifoReadResult),
        std::move(counters),
        std::move(rates),
        prevElapsed
    );
}

// clang-format off
const std::string CounterRates::TcmPattern = 
    std::format(
        R"(READ_INTERVAL,({}),({})s\n)" // 1, 2
        R"(FIFO_STATE,({}),({})\n)" // 3, 4
        R"(FIFO_READ_RESULT,({})\n)" // 5
        R"(COUNTERS,(?:{}|-)\n)" // 6-20
        R"(RATES,(?:{}|-)\n)" // 21-35
        R"(PREV_ELAPSED,({})ms\n)" // 36
        R"(Executed:([^\n]*))", // 37
        STR, FLT,
        STR, DEC,
        STR,
        repeat(std::format("({})", DEC), ",", 15),
        repeat(std::format("({})", FLT), ",", 15),
        FLT
    );

const std::string CounterRates::PmPattern = 
    std::format(
        R"(READ_INTERVAL,({}),({})s\n)" // 1, 2
        R"(FIFO_STATE,({}),({})\n)" // 3, 4
        R"(FIFO_READ_RESULT,({})\n)" // 5
        R"(COUNTERS,(?:{}|-)\n)" // 6-29
        R"(RATES,(?:{}|-)\n)" // 30-44
        R"(PREV_ELAPSED,({})ms\n)" // 45
        R"(Executed:([^\n]*))", // 46
        STR, FLT,
        STR, DEC,
        STR,
        repeat(std::format("({})", DEC), ",", 24),
        repeat(std::format("({})", FLT), ",", 24),
        FLT
    );
// clang-format on

CounterRates::CounterRates(Board board) :
    TrackingTest(
        board.name() + " COUNTER_RATES TRACKER",
        MapiHandler::get(topic(board, "COUNTER_RATES")),
        0.5,
        0,
        board.isTcm() ? TcmPattern : PmPattern,
        nullptr
    ),
    m_board(board),
    m_valueTracker(m_testName, board.isTcm() ? 15 : 24, m_expectedInterval) {
    m_valueValidator =
        std::ref(m_valueTracker); // avoid initialization order problems
}

Result<void> CounterRates::ValueTracker::operator()(std::smatch match) {
    Response res;
    TRY_ASSIGN(
        Response::fromMatch(testName, std::move(match), numberOfCounters),
        res
    );

    if ((res.readIntervalState != "OK" && res.readIntervalState != "CHANGED")
        || res.readIntervalSeconds != 2 * currentInterval) {
        return err(
            "Invalid read interval: status {}, {}s",
            res.readIntervalState,
            res.readIntervalSeconds
        );
    }

    if (res.fifoState != "SINGLE" && res.fifoState != "MULTIPLE"
        && res.fifoState != "PARTIAL" && res.fifoState != "EMPTY"
        && res.fifoState != "OUTDATED") {
        return err("Invalid FIFO state: {}", res.fifoState);
    }

    if (res.fifoState == "FAILURE" || res.fifoState == "_INTERNAL_ERROR") {
        return err("No FIFO readout: {}", res.fifoState);
    }

    if (res.fifoReadResult == "SUCCESS") {
        auto now = std::chrono::steady_clock::now();
        if (lastTime) {
            std::chrono::duration<double> elapsed = now - *lastTime;
            newValuesInterval.tick(elapsed.count());
        }
        Logger::debug(testName, "New rates received");
        lastTime = now;
    }

    if (res.hasRates()) {
        Logger::debug(testName, "Storing new rates");
        for (size_t i = 0; i < 15; i++) {
            rates[i].tick(res.rates[i]);
        }
    }

    elapsed.tick(res.prevElapsed);
    return {};
}

void CounterRates::logSummary() const {
    auto rates =
        m_valueTracker.rates | std::ranges::views::transform([](const auto& r) {
            return std::format("\n{:.3f}±{:.3f}", r.mean(), r.stddev());
        });
    std::string ratesStr = "Rates";
    for (auto it = rates.begin(); it < rates.end(); it++) {
        ratesStr += *it;
    }
    Logger::info(m_testName, "{}", ratesStr);

    Logger::info(
        m_testName,
        "New values interval: {:.3f}±{:.3f}s | Elapsed: {:.3f}±{:.3f}ms",
        m_valueTracker.newValuesInterval.mean(),
        m_valueTracker.newValuesInterval.stddev(),
        m_valueTracker.elapsed.mean(),
        m_valueTracker.elapsed.stddev()
    );
}

CounterRates::ValueTracker::ValueTracker(
    std::string testName,
    size_t numberOfCounters,
    double currentInterval
) :
    testName(testName),
    numberOfCounters(numberOfCounters),
    currentInterval(currentInterval),
    rates(numberOfCounters) {}

void CounterRates::resetCounters() {
    Logger::info(m_testName, "Performing reset");
    m_mapi->sendCommand("RESET");
}

std::string CounterRates::getBadChannelMap(
    TesterConfig::BadChannelMapConfig cfg
) {
    if (m_board.isTcm()) {
        return std::format(
            "ERROR: requested bad channel map for {}",
            m_board.name()
        );
    }

    std::string response;
    for (uint32_t chIdx = 0; chIdx < 12; chIdx++) {
        utils::Channel ch(m_board, chIdx);
        bool result = cfg.validateValue(ch, m_valueTracker.rates[2. * chIdx + 1].mean()); // TRG rate
        response += std::format("{},{}\n", ch.toStr(), result);
    }

    return response;
}

} // namespace tests
