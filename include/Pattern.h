#pragma once

#include <cmath>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

static constexpr std::string_view FLT = R"(([+-]?(?:\d+\.\d*|\.\d+|\d+)))";
static constexpr std::string_view DEC = R"(([+-]?\d+))";
static constexpr std::string_view HEX = R"(([+-]?0[xX][0-9a-fA-F]+))";
static constexpr std::string_view STR = R"(([^,]+))";

class Pattern;

enum class Format { Flt, Dec, Str };

struct Val {
    std::string name;
    Format fmt;
    std::variant<double, std::string> val;
    std::optional<std::variant<double, std::string>> expectedVal;

    Val(std::string name,
        Format fmt,
        std::optional<std::variant<double, std::string>> expectedVal) :
        name(name),
        fmt(fmt),
        val(fmt == Format::Str ? std::variant<double, std::string>("")
                               : std::variant<double, std::string>(NAN)),
        expectedVal(expectedVal) {}

    bool isEqual() const {
        return !expectedVal || (val == *expectedVal);
    }
};

class PatternBuilder {
    friend class Pattern;

    std::string m_pattern;

    std::vector<Val> m_refs;

    bool replaceFirstPlaceholder(const std::string_view s);

  public:
    PatternBuilder(std::string pattern);

    PatternBuilder& flt(std::string name);
    PatternBuilder& dec(std::string name);
    PatternBuilder& str(std::string name);

    Pattern build();
};

class Pattern {
    std::regex m_re;
    std::vector<Val> m_refs;
    std::smatch m_match;

  public:
    Pattern(PatternBuilder&& b) :
        m_re(std::move(b.m_pattern)),
        m_refs(std::move(b.m_refs)) {}

    bool fullMatch(const std::string& str);
    void parseValues();
};
