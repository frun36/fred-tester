#include "CommandTest.h"
#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Parameters: public TestSuite {
  private:
    std::string boardName;

    CommandTest writeLSB15(std::string boardName) {
        return TestBuilder("{} WRITE", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0x0F")
            .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)", utils::FLT)
            .withValueValidator([](auto match) -> Result<void> {
                if (std::stod(match[1]) == 15) {
                    return {};
                } else {
                    return err("Read value {}", match[1].str());
                }
            })
            .timeout(0.2)
            .expectOk()
            .build();
    }

    CommandTest writeElectronicMSB255(std::string boardName) {
        return TestBuilder("{} WRITE_ELECTRONIC", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command("GBT_EMULATED_TRIGGERS_PATTERN_MSB,WRITE_ELECTRONIC,0xFF")
            .pattern(R"(GBT_EMULATED_TRIGGERS_PATTERN_MSB,({})\n)", utils::FLT)
            .withValueValidator([](auto match) -> Result<void> {
                if (std::stod(match[1]) == 255) {
                    return {};
                } else {
                    return err("Read value {}", match[1].str());
                }
            })
            .timeout(0.2)
            .expectOk()
            .build();
    }

    CommandTest doubleRead(
        std::string boardName,
        std::pair<double, double> expected
    ) {
        return TestBuilder("{} READ", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command(
                "GBT_EMULATED_TRIGGERS_PATTERN_MSB,READ\n"
                "GBT_EMULATED_TRIGGERS_PATTERN_LSB,READ\n"
            )
            .pattern(
                R"(GBT_EMULATED_TRIGGERS_PATTERN_(MSB|LSB),({})\n)"
                R"(GBT_EMULATED_TRIGGERS_PATTERN_(MSB|LSB),({})\n)",
                utils::FLT,
                utils::FLT
            )
            .withValueValidator([expected](auto match) -> Result<void> {
                double msb = std::stod(match[1] == "MSB" ? match[2] : match[4]);
                double lsb = std::stod(match[1] == "LSB" ? match[2] : match[4]);
                if (msb != expected.first) {
                    return err("Read MSB value {} ≠ {}", msb, expected.first);
                }
                if (lsb != expected.second) {
                    return err("Read LSB value {} ≠ {}", lsb, expected.second);
                }
                return {};
            })
            .timeout(0.2)
            .expectOk()
            .build();
    }

    CommandTest writeOutOfRangeLSB(std::string boardName) {
        return TestBuilder("{} WRITE OUT OF RANGE", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,-36")
            .pattern(
                R"((?:.*:\s)+attempted to write a value outside the valid range [{}; {}] - value {})",
                utils::DEC,
                utils::DEC,
                utils::DEC
            )
            .withoutValueValidator()
            .timeout(0.2)
            .expectError()
            .build();
    }

    CommandTest readNonexistent(std::string boardName) {
        return TestBuilder("{} READ NONEXISTENT", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command("HELOU_NONEXISTENT_PARAM,READ")
            .pattern(
                R"((?:.*:\s)+Parameter {} not found on the board.)",
                utils::STR,
                utils::STR
            )
            .withoutValueValidator()
            .timeout(0.2)
            .expectError()
            .build();
    }

    CommandTest doubleWrite(
        std::string boardName,
        std::pair<double, double> values
    ) {
        return TestBuilder("{} READ", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command(
                std::format(
                    "GBT_EMULATED_TRIGGERS_PATTERN_MSB,WRITE,{}\n"
                    "GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,{}\n",
                    values.first,
                    values.second
                )
            )
            .pattern(
                R"(GBT_EMULATED_TRIGGERS_PATTERN_(LSB|MSB),({})\n)"
                R"(GBT_EMULATED_TRIGGERS_PATTERN_(LSB|MSB),({})\n)",
                utils::FLT,
                utils::FLT
            )
            .withValueValidator([values](auto match) -> Result<void> {
                double msb = std::stod(match[1] == "MSB" ? match[2] : match[4]);
                double lsb = std::stod(match[1] == "LSB" ? match[2] : match[4]);
                if (msb != values.first) {
                    return err("Failed to write MSB ({} ≠ {})", msb, values.first);
                }
                if (lsb != values.second) {
                    return err("Failed to write LSB ({} ≠ {})", lsb, values.second);
                }
                return {};
            })
            .timeout(0.2)
            .expectOk()
            .build();
    }

    CommandTest doubleWriteOutOfRangeMSB(std::string boardName) {
        return TestBuilder("{} READ", boardName)
            .mapiName(utils::topic(boardName, "PARAMETERS"))
            .command(
                "GBT_EMULATED_TRIGGERS_PATTERN_MSB,WRITE,-36\n"
                "GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0x00\n"
            )
            .pattern(
                R"({}: attempted to write a value outside the valid range [{}; {}] - value {})",
                utils::STR,
                utils::DEC,
                utils::DEC,
                utils::DEC
            )
            .withoutValueValidator()
            .timeout(0.2)
            .expectError()
            .build();
    }

  public:
    Parameters(std::string boardName) :
        TestSuite({
            writeLSB15(boardName),
            writeElectronicMSB255(boardName),
            writeOutOfRangeLSB(boardName),
            doubleRead(boardName, {0xFF, 0x0F}),
            readNonexistent(boardName),
            doubleWrite(boardName, {0xBE, 0xEF}),
            doubleWriteOutOfRangeMSB(boardName),
            doubleRead(boardName, {0xBE, 0xEF}),
        }) {}
};

} // namespace tests
