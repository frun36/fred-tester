#include "tests/TestParameters.h"

#include <format>
#include <regex>

#include "CommandTest.h"
#include "Result.h"
#include "utils.h"

namespace tests {

CommandTest TestParameters::generateTestOk(
    utils::Board board,
    std::string testName,
    std::vector<ParameterOperation> operations
) {
    std::string command;
    for (auto& op : operations) {
        command += std::format("{},{}", op.name, op.type);
        if (op.type == "WRITE" || op.type == "WRITE_ELECTRONIC") {
            command += std::format(",{}", op.value);
        }
        command += "\n";
    }

    std::string pattern;
    for (size_t i = 0; i < operations.size(); i++) {
        pattern += std::format("([A-Z0-9_]+),({})\\n", utils::FLT);
    }

    auto valueValidator =
        [operations](std::smatch match) mutable -> Result<void> {
        for (size_t i = 1; i < match.size() - 1; i += 2) {
            auto it = std::find_if(
                operations.begin(),
                operations.end(),
                [i, &match](auto op) { return op.name == match[i].str(); }
            );
            if (it == operations.end())
                return err("Parameter {} was not requested", match[i].str());
            // cannot validate write electronic since we don't know the equation
            if (it->type == "WRITE"
                && it->value != std::stod(match[i + 1].str()))
                return err(
                    "{}: expected {}, got {}",
                    it->name,
                    it->value,
                    match[i + 1].str()
                );
            it->validation = true;
        }

        for (auto& op : operations) {
            if (!op.validation)
                return err(
                    "No response returned for parameter {}\n{}",
                    op.name,
                    match[0].str()
                );
        }

        return {};
    };

    return TestBuilder("{} {}", board.name(), testName)
        .mapiName(utils::topic(board, "PARAMETERS"))
        .command(command)
        .pattern("{}", pattern)
        .withValueValidator(valueValidator)
        .timeout(0.2)
        .expectOk()
        .build();
}

CommandTest TestParameters::generateTestErr(
    utils::Board board,
    std::string testName,
    std::vector<ParameterOperation> operations,
    std::string errorMsg
) {
    std::string command;
    for (auto& op : operations) {
        command += std::format("{},{}", op.name, op.type);
        if (op.type == "WRITE" || op.type == "WRITE_ELECTRONIC") {
            command += std::format(",{}", op.value);
        }
        command += "\n";
    }

    return TestBuilder("{} {}", board.name(), testName)
        .mapiName(utils::topic(board, "PARAMETERS"))
        .command(command)
        .pattern(".*{}.*", errorMsg)
        .withoutValueValidator()
        .timeout(0.2)
        .expectError()
        .build();
}

TestParameters::TestParameters() :
    TestSuite({
        generateTestOk(utils::TCM0, "EmptyRequest", {}),
        generateTestErr(
            utils::TCM0,
            "NonexistentParamReadFailed",
            {{"TEST_NONEXISTENT", "READ", 0.}},
            "not found"
        ),
        generateTestErr(
            utils::TCM0,
            "NonexistentParamWriteFailed",
            {{"TEST_NONEXISTENT", "WRITE", 0.}},
            "not found"
        ),
        generateTestErr(
            utils::TCM0,
            "WriteOutOfRangeUnsigned",
            {{"TEST_A", "WRITE", -0x0D}},
            "outside the valid range"
        ),
        generateTestErr(
            utils::TCM0,
            "WriteOutOfRangeSigned",
            {{"TEST_B", "WRITE", 0xFF}},
            "outside the valid range"
        ),
        generateTestOk(
            utils::TCM0,
            "WriteInitialValues",
            {{"TEST_A", "WRITE", 0x0D},
             {"TEST_B", "WRITE", -1},
             {"TEST_C", "WRITE", 0xF0},
             {"TEST_D", "WRITE", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_AB",
            {{"TEST_A", "READ", 0x0D}, {"TEST_B", "READ", -1}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_AllParams",
            {{"TEST_READONLY", "READ", 0xA11CEF17},
             {"TEST_C", "READ", 0xF0},
             {"TEST_B", "READ", -1},
             {"TEST_D", "READ", 0xC0FFEE},
             {"TEST_A", "READ", 0x0D}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_AC",
            {{"TEST_A", "READ", 0x0D}, {"TEST_C", "READ", 0xF0}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_BD",
            {{"TEST_B", "READ", -1}, {"TEST_D", "READ", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_CReadonly",
            {{"TEST_READONLY", "READ", 0xA11CEF17}, {"TEST_C", "READ", 0xF0}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_ABC",
            {{"TEST_A", "READ", 0x0D},
             {"TEST_B", "READ", -1},
             {"TEST_C", "READ", 0xF0}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_BCD",
            {{"TEST_B", "READ", -1},
             {"TEST_C", "READ", 0xF0},
             {"TEST_D", "READ", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_CDR",
            {{"TEST_READONLY", "READ", 0xA11CEF17},
             {"TEST_C", "READ", 0xF0},
             {"TEST_D", "READ", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_ABD",
            {{"TEST_A", "READ", 0x0D},
             {"TEST_B", "READ", -1},
             {"TEST_D", "READ", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiRead_ACD",
            {{"TEST_A", "READ", 0x0D},
             {"TEST_C", "READ", 0xF0},
             {"TEST_D", "READ", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWrite_AB",
            {{"TEST_A", "WRITE", 0xEF}, {"TEST_B", "WRITE", 0x6E}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWrite_AC",
            {{"TEST_A", "WRITE", 0x0D}, {"TEST_C", "WRITE", 12}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWrite_BD",
            {{"TEST_B", "WRITE", 0x40}, {"TEST_D", "WRITE", 0xCAFE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWrite_ABC",
            {{"TEST_A", "WRITE", 0xEF},
             {"TEST_B", "WRITE", 0x6E},
             {"TEST_C", "WRITE", 0xACE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWrite_BCD",
            {{"TEST_B", "WRITE", 0x60},
             {"TEST_C", "WRITE", 0x0E},
             {"TEST_D", "WRITE", 0xC0FFEE}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiWriteWithElectronic_ACD",
            {{"TEST_A", "WRITE", 0xAB},
             {"TEST_C", "WRITE_ELECTRONIC", 0x66},
             {"TEST_D", "WRITE", 0xBEEFF00D}}
        ),
        generateTestOk(
            utils::TCM0,
            "SuccessfulMultiReadAfterWriteElectronic_AC",
            {{"TEST_A", "READ", 0xAB}, {"TEST_C", "READ", 0xCC}}
        ),
        generateTestErr(
            utils::TCM0,
            "Write_CDR_FailsDueToReadOnly",
            {{"TEST_C", "WRITE", 0xF0},
             {"TEST_D", "WRITE", 0xC0FFEE},
             {"TEST_READONLY", "WRITE", 0xAAAA}},
            "read-only"
        ),
    }) {}

} // namespace tests
