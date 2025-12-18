#pragma once

#include <stdexcept>

#include "TrackingTest.h"
#include "tests/ParameterValidator.h"
#include "utils.h"

class ScStatus: public TrackingTest {
  private:
    ParameterValidator m_validator;

  public:
    ScStatus(std::string link, uint32_t boardId) :
        TrackingTest(
            "SC_STATUS TRACKER",
            MapiHandler::get(
                utils::topic(
                    *utils::Board::fromName(
                        std::format("VIRTUAL_SC{}", boardId)
                    ),
                    "SC_STATUS"
                )
            ),
            1.0,
            0,
            std::format(R"((?:{},{}\n)+)", utils::STR, utils::FLT),
            [this](auto smatch) {
                return m_validator.validate(smatch[0].str());
            }
        ) {
        m_validator.eq("TRANSLATOR_ERR", 0).eq("GBT_ERR", 0).eq("IPBUS_ERR", 0);
        if (link == "GBT")
            m_validator.eq("LINK_USED", 1);
        else if (link == "IPBUS")
            m_validator.eq("LINK_USED", 0);
        else
            throw std::runtime_error("Unexpected SC link name: " + link);

        if (boardId > 1)
            throw std::runtime_error("Unexpected boardId for VIRTUAL_SC");
    }

    ScStatus(ScStatus&& other) :
        TrackingTest(std::move(other)),
        m_validator(std::move(other.m_validator)) {
        m_valueValidator = [this](auto smatch) {
            return m_validator.validate(smatch[0].str());
        };
    }

    ScStatus& operator=(ScStatus&& other) {
        if (this != &other) {
            TrackingTest::operator=(std::move(other));
            m_validator = std::move(other.m_validator);
            m_valueValidator = [this](auto smatch) {
                return m_validator.validate(smatch[0].str());
            };
        }
        return *this;
    }

    ScStatus(const ScStatus&) = delete;
    ScStatus& operator=(const ScStatus&) = delete;
};
