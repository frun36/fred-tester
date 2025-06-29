#include "CommandTest.h"
#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Parameters: public TestSuite {
  public:
    Parameters(std::string boardName) :
        TestSuite(
            {TestBuilder("{} WRITE", boardName)
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0x0F")
                 .pattern(
                     R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)",
                     utils::FLT
                 )
                 .withValueValidator([](auto match) -> Result<void> {
                     if (std::stod(match[1]) == 15) {
                         return {};
                     } else {
                         return err("Read value {}", match[1].str());
                     }
                 })
                 .timeout(0.2)
                 .expectOk()
                 .build(),

             TestBuilder("{} READ", boardName)
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,READ")
                 .pattern(
                     R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)",
                     utils::FLT
                 )
                 .withValueValidator([](auto match) -> Result<void> {
                     if (std::stod(match[1]) == 15) {
                         return {};
                     } else {
                         return err("Read value {}", match[1].str());
                     }
                 })
                 .timeout(0.2)
                 .expectOk()
                 .build(),

             TestBuilder("{} WRITE_ELECTRONIC", boardName)
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE_ELECTRONIC,0xFF")
                 .pattern(
                     R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)",
                     utils::FLT
                 )
                 .withValueValidator([](auto match) -> Result<void> {
                     if (std::stod(match[1]) == 255) {
                         return {};
                     } else {
                         return err("Read value {}", match[1].str());
                     }
                 })
                 .timeout(0.2)
                 .expectOk()
                 .build(),

             TestBuilder("{} WRITE_ELECTRONIC", boardName)
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,-36")
                 .pattern(
                     R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)",
                     utils::FLT
                 )
                 .withValueValidator([](auto match) -> Result<void> {
                     if (std::stod(match[1]) == 255) {
                         return {};
                     } else {
                         return err("Read value {}", match[1].str());
                     }
                 })
                 .timeout(0.2)
                 .expectOk()
                 .build(),

             TestBuilder("{} READ", boardName)
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,READ")
                 .pattern(
                     R"(GBT_EMULATED_TRIGGERS_PATTERN_LSB,({})\n)",
                     utils::FLT
                 )
                 .withValueValidator([](auto match) -> Result<void> {
                     if (std::stod(match[1]) == 255) {
                         return {};
                     } else {
                         return err("Read value {}", match[1].str());
                     }
                 })
                 .timeout(0.2)
                 .expectOk()
                 .build()}
        ) {}
};

} // namespace tests
