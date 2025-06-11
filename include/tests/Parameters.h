#include "Test.h"
#include "TestSuite.h"
#include "utils.h"

namespace tests {

class Parameters: public TestSuite {
  public:
    Parameters(std::string boardName) :
        TestSuite(
            {TestBuilder(boardName + " WRITE")
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

             TestBuilder(boardName + " READ")
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

             TestBuilder(boardName + " WRITE_ELECTRONIC")
                 .mapiName(utils::topic(boardName, "PARAMETERS"))
                 .command("GBT_EMULATED_TRIGGERS_PATTERN_LSB,WRITE,0xFF")
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

             TestBuilder(boardName + " READ")
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
