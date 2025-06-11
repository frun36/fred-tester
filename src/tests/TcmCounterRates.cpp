#include "tests/TcmCounterRates.h"

#include <utility>

#include "Result.h"
#include "utils.h"

using namespace utils;

namespace tests {

TcmCounterRates::Response::Response(
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

Result<TcmCounterRates::Response> TcmCounterRates::Response::fromMatch(
    std::smatch match
) {
    if (match.size() < 37) {
        return Error("Match contains too few matched groups: {}", match.size());
    }

    std::string readIntervalState = match[1].str();

    double readIntervalSeconds;
    TRY_ASSIGN(parseDouble(match[2]), readIntervalSeconds);

    std::string fifoState = match[3].str();

    uint32_t fifoLoad;
    TRY_ASSIGN(parseInt(match[4]), fifoLoad);

    std::string fifoReadResult = match[5].str();

    std::vector<uint32_t> counters(15);
    if (match[6] == "-") {
        counters.clear();
    } else {
        for (size_t i = 0; i < 15; i++) {
            TRY_ASSIGN(parseInt(match[6 + i]), counters[i]);
        }
    }

    std::vector<double> rates(15);
    if (match[21] == "-") {
        rates.clear();
    } else {
        for (size_t i = 0; i < 15; i++) {
            TRY_ASSIGN(parseDouble(match[21 + i]), rates[i]);
        }
    }

    double prevElapsed;
    TRY_ASSIGN(parseDouble(match[36]), prevElapsed);

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

TcmCounterRates::TcmCounterRates() :
    TrackingTest(
        "TCM COUNTER_RATES tracker",
        MapiHandler::get(Topic("TCM0", "COUNTER_RATES")),
        0.5,
        // clang-format off
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
        ),
        // clang-format on
        nullptr
    ),
    m_valueTracker(m_testName) {
    m_valueValidator =
        std::ref(m_valueTracker); // avoid initialization order problems
}

Result<void> TcmCounterRates::ValueTracker::operator()(std::smatch match) {
    Response res;
    TRY_ASSIGN(Response::fromMatch(std::move(match)), res);

    if (res.readIntervalState != "OK" || res.readIntervalSeconds != 1.) {
        return Error(
            "Invalid read interval: status {}, {}s",
            res.readIntervalState,
            res.readIntervalSeconds
        );
    }

    if (res.fifoState != "SINGLE" && res.fifoState != "MULTIPLE"
        && res.fifoState != "PARTIAL" && res.fifoState != "EMPTY") {
        return Error("Invalid FIFO state: {}", res.fifoState);
    }

    if (res.fifoState == "FAILURE" || res.fifoState == "_INTERNAL_ERROR") {
        return Error("No FIFO readout: {}", res.fifoState);
    }

    if (res.fifoReadResult == "SUCCESS") {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        newValuesInterval.tick(elapsed.count());
        Logger::debug(
            m_testName,
            "New rates received, last: {}s ago",
            elapsed.count()
        );
        lastTime = now;
    }

    if (res.hasRates()) {
        Logger::debug(m_testName, "Storing new rates");
        for (size_t i = 0; i < 15; i++) {
            rates[i].tick(res.rates[i]);
        }
    }

    elapsed.tick(res.prevElapsed);
    return {};
}

std::string TcmCounterRates::ValueTracker::summary() const {
    // clang-format off
    return std::format(
        "Rates: {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f}, {:.3f}±{:.3f} | New values interval: {:.3f}±{:.3f}s | Elapsed: {:.3f}±{:.3f}ms",
        rates[0].mean(), rates[0].stddev(),
        rates[1].mean(), rates[1].stddev(),
        rates[2].mean(), rates[2].stddev(),
        rates[3].mean(), rates[3].stddev(),
        rates[4].mean(), rates[4].stddev(),
        rates[5].mean(), rates[5].stddev(),
        rates[6].mean(), rates[6].stddev(),
        rates[7].mean(), rates[7].stddev(),
        rates[8].mean(), rates[8].stddev(),
        rates[9].mean(), rates[9].stddev(),
        rates[10].mean(), rates[10].stddev(),
        rates[11].mean(), rates[11].stddev(),
        rates[12].mean(), rates[12].stddev(),
        rates[13].mean(), rates[13].stddev(),
        rates[14].mean(), rates[14].stddev(),
        newValuesInterval.mean(), newValuesInterval.stddev(),
        elapsed.mean(), elapsed.stddev()
    );
    // clang-format on
}

void TcmCounterRates::logSummary() const {
    Logger::info(m_testName, "{}", m_valueTracker.summary());
}

TcmCounterRates::ValueTracker::ValueTracker(std::string testName) :
    m_testName(testName) {}
} // namespace tests
