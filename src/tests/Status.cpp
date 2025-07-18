#include "tests/Status.h"

#include <expected>
#include <format>
#include <ranges>
#include <unordered_map>

#include "CommandTest.h"
#include "MapiHandler.h"

namespace tests {

Status::Status(utils::Board board, std::vector<utils::Board> connectedBoards) :
    TrackingTest(
        board.name() + " STATUS TRACKER",
        MapiHandler::get(topic(board, "STATUS")),
        1.0,
        0,
        std::format(R"((?:{},{}\n)+)", utils::STR, utils::FLT),
        [this](auto smatch) { return m_validator.validate(smatch[0].str()); }
    ) {
    m_validator.eq("IS_BOARD_OK", 1.)
        .eq("GBT_RX_PHASE_ERROR", 0.)
        .eq("GBT_RESET_READOUT_FSM", 0.)
        .eq("GBT_FIFO_EMPTY_SLCT_WHILE_READ", 0.)
        .eq("GBT_FIFO_SLCT_NOT_EMPTY_ON_RUN_START", 0.)
        .eq("GBT_FIFO_CNT_PACK_NOT_EMPTY_ON_RUN_START", 0.)
        .eq("GBT_FIFO_TRG_NOT_EMPTY_ON_RUN_START", 0.)
        .eq("GBT_FIFO_TRG_FULL", 0.)
        .eq("GBT_FIFO_DATA_NOT_EMPTY_ON_RUN_START", 0.)
        .eq("GBT_FIFO_HEADER_NOT_EMPTY_ON_RUN_START", 0.)
        .eq("GBT_FIFO_TCM_DATA_FULL", 0.)
        .eq("GBT_BC_SYNC_LOST_IN_RUN", 0.);

    if (board.isTcm()) {
        m_validator.eq("COUNTER_READ_INTERVAL", 4.)
            .neq("BOARD_STATUS_PLL_LOCK_A", 0.)
            .neq("BOARD_STATUS_PLL_LOCK_C", 0.)
            .eq("SIDE_A_DELAY_RANGE_ERROR", 0.)
            .eq("SIDE_C_DELAY_RANGE_ERROR", 0.)
            .eq("SIDE_A_MASTER_LINK_DELAY_ERROR", 0.)
            .eq("SIDE_C_MASTER_LINK_DELAY_ERROR", 0.)
            .eq("SIDE_A_READY_BIT_CHANGES_STATE", 0.)
            .eq("SIDE_C_READY_BIT_CHANGES_STATE", 0.);

        for (auto cb : connectedBoards) {
            if (cb.isPm()) {
                m_validator.neq(std::format("{}_LINK_OK", cb.name()), 0.)
                    .eq(std::format(
                            "SIDE_{}_SYNC_ERROR_CHANNEL_{}{}",
                            cb.name()[2],
                            cb.name()[2],
                            cb.name()[3]
                        ),
                        0.);
            }
        }
    } else { // PM
        m_validator.neq("MAIN_PLL_LOCKED", 0.)
            .neq("TDC1_PLL_LOCKED", 0.)
            .neq("TDC2_PLL_LOCKED", 0.)
            .neq("TDC3_PLL_LOCKED", 0.)
            .eq("TDC1_SYNC_ERROR", 0.)
            .eq("TDC2_SYNC_ERROR", 0.)
            .eq("TDC3_SYNC_ERROR", 0.)
            .neq("LAST_RESTART_REASON", 2.);
    }
}

Status::Status(Status&& other) :
    TrackingTest(std::move(other)),
    m_validator(std::move(other.m_validator)) {
    m_valueValidator = [this](auto smatch) {
        return m_validator.validate(smatch[0].str());
    };
}

Status& Status::operator=(Status&& other) {
    if (this != &other) {
        TrackingTest::operator=(std::move(other));
        m_validator = std::move(other.m_validator);
        m_valueValidator = [this](auto smatch) {
            return m_validator.validate(smatch[0].str());
        };
    }
    return *this;
}

Status::ValueValidator& Status::ValueValidator::eq(
    std::string name,
    double value
) {
    expectedValues[name] = {value, true};
    return *this;
}

Status::ValueValidator& Status::ValueValidator::neq(
    std::string name,
    double value
) {
    expectedValues[name] = {value, false};
    return *this;
}

Result<void> Status::ValueValidator::validate(std::string str) {
    std::unordered_map<std::string, double> statusValues;
    statusValues.reserve(1024);
    std::string errorReport;
    for (auto line : std::views::split(str, '\n')) {
        if (line.empty())
            continue;
        auto parts = std::views::split(line, ',');
        auto it = parts.begin();
        std::string_view name(*it++);
        std::string_view valueStr(*it++);
        auto valueRes = utils::parseDouble(std::string(valueStr));
        if (!valueRes) {
            errorReport += std::format("{}: {}\n", name, valueRes.error());
        }

        statusValues[std::string(name)] = *valueRes;
    }

    for (auto [name, expected] : expectedValues) {
        auto statusIt = statusValues.find(name);
        if (statusIt == statusValues.end())
            errorReport += std::format(
                "Parameter {} not found in STATUS response\n",
                name
            );

        double statusValue = statusIt->second;
        if ((statusValue == expected.value) != expected.eq)
            errorReport += std::format(
                "{} = {}; expected {} {}\n",
                name,
                statusValue,
                expected.eq ? "=" : "≠",
                expected.value
            );
    }

    if (errorReport.empty()) {
        return {};
    } else {
        errorReport.pop_back();
        return err("{}", errorReport);
    }
}

} // namespace tests
