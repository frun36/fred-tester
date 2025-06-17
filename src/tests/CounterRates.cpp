#include "tests/CounterRates.h"

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
    if (*it == "-") {
        Logger::warning(testName, "No counters");
        counters.clear();
    } else {
        for (size_t i = 0; i < numberOfCounters; i++) {
            TRY_ASSIGN(parseInt(*it++), counters[i]);
        }
    }

    std::vector<double> rates(numberOfCounters);
    if (*it == "-") {
        rates.clear();
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
        R"(COUNTERS,({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-)\n)" // 6-20
        R"(RATES,({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-)\n)" // 21-35
        R"(PREV_ELAPSED,({})ms\n)" // 36
        R"(Executed:([^\n]?))", // 37
        STR, FLT,
        STR, DEC,
        STR,
        DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, 
        FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, 
        FLT
    );

const std::string CounterRates::PmPattern = 
    std::format(
        R"(READ_INTERVAL,({}),({})s\n)" // 1, 2
        R"(FIFO_STATE,({}),({})\n)" // 3, 4
        R"(FIFO_READ_RESULT,({})\n)" // 5
        R"(COUNTERS,({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-)\n)" // 6-29
        R"(RATES,({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-),({}|-)\n)" // 30-44
        R"(PREV_ELAPSED,({})ms\n)" // 45
        R"(Executed:([^\n]?))", // 46
        STR, FLT,
        STR, DEC,
        STR,
        DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, DEC, 
        FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, FLT, 
        FLT
    );
// clang-format on

CounterRates::CounterRates(std::string boardName) :
    TrackingTest(
        boardName + " COUNTER_RATES tracker",
        MapiHandler::get(topic(boardName, "COUNTER_RATES")),
        0.5,
        type(boardName) == "TCM" ? TcmPattern : PmPattern,
        nullptr
    ),
    m_valueTracker(m_testName, type(boardName) == "TCM" ? 15 : 24) {
    m_valueValidator =
        std::ref(m_valueTracker); // avoid initialization order problems
}

Result<void> CounterRates::ValueTracker::operator()(std::smatch match) {
    Response res;
    TRY_ASSIGN(Response::fromMatch(testName, std::move(match), numberOfCounters), res);

    if (res.readIntervalState != "OK" || res.readIntervalSeconds != 1.) {
        return err(
            "Invalid read interval: status {}, {}s",
            res.readIntervalState,
            res.readIntervalSeconds
        );
    }

    if (res.fifoState != "SINGLE" && res.fifoState != "MULTIPLE"
        && res.fifoState != "PARTIAL" && res.fifoState != "EMPTY") {
        return err("Invalid FIFO state: {}", res.fifoState);
    }

    if (res.fifoState == "FAILURE" || res.fifoState == "_INTERNAL_ERROR") {
        return err("No FIFO readout: {}", res.fifoState);
    }

    if (res.fifoReadResult == "SUCCESS") {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        newValuesInterval.tick(elapsed.count());
        Logger::debug(
            testName,
            "New rates received, last: {}s ago",
            elapsed.count()
        );
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
            return std::format(" {:.3f}±{:.3f}", r.mean(), r.stddev());
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
    size_t numberOfCounters
) :
    testName(testName),
    numberOfCounters(numberOfCounters),
    rates(numberOfCounters) {}

void CounterRates::resetCounters() {
    m_mapi->sendCommand("RESET");
    Logger::info(m_testName, "Performing reset");

}

} // namespace tests
