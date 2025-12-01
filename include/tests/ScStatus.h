#pragma once

#include <stdexcept>

#include "TrackingTest.h"
#include "tests/ParameterValidator.h"
#include "utils.h"

class ScStatus: public TrackingTest {
  private:
    ParameterValidator m_validator;

  public:
    ScStatus(std::string link) :
        TrackingTest(
            "SC_STATUS TRACKER",
            MapiHandler::get(
                utils::topic({utils::Board::BoardName::TCM1}, "SC_STATUS")
            ),
            1.0,
            0,
            std::format(R"((?:{},{}\n)+)", utils::STR, utils::FLT),
            [this](auto smatch) {
                return m_validator.validate(smatch[0].str());
            }
        ) {
        m_validator.eq("TRANSLATOR_ERR", 0);
        if (link == "GBT") {
            m_validator.eq("LINK_USED", 0).eq("GBT_ERR", 0);
        } else if (link == "IPBUS") {
            m_validator.eq("LINK_USED", 0).eq("IPBUS_ERR", 0);
        } else {
            throw std::runtime_error("Unexpected SC link name: " + link);
        }
    }

    ScStatus(ScStatus&& other) = default;
    ScStatus& operator=(ScStatus&& other) = default;

    ScStatus(const ScStatus&) = delete;
    ScStatus& operator=(const ScStatus&) = delete;
};
