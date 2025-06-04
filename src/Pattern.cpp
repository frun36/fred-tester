#include "Pattern.h"
#include <memory>

bool PatternBuilder::replaceFirstPlaceholder(const std::string_view s) {
    size_t pos = m_pattern.find("{}");
    if (pos == std::string::npos) {
        return false;
    }

    m_pattern.replace(pos, 2, s);
    return true;
}

PatternBuilder& PatternBuilder::flt(std::string name) {
    if (!replaceFirstPlaceholder(FLT)) {
        throw std::runtime_error("Couldn't find argument position");
    }
    m_refs.emplace_back(name, Format::Flt, std::nullopt);
    return *this;
}

PatternBuilder& PatternBuilder::dec(std::string name) {
    if (!replaceFirstPlaceholder(DEC)) {
        throw std::runtime_error("Couldn't find argument position");
    }
    m_refs.emplace_back(name, Format::Dec, std::nullopt);
    return *this;
}

PatternBuilder& PatternBuilder::str(std::string name) {
    if (!replaceFirstPlaceholder(STR)) {
        throw std::runtime_error("Couldn't find argument position");
    }
    m_refs.emplace_back(name, Format::Str, std::nullopt);
    return *this;
}

Pattern PatternBuilder::build() {
    while (replaceFirstPlaceholder(STR))
        ;

    return Pattern(std::move(*this));
}

bool Pattern::fullMatch(const std::string& str) {
    return std::regex_match(str, m_match, m_re);
}

void Pattern::parseValues() {
    for (size_t i = 0; i < m_refs.size(); i++) {
        switch (m_refs[i].fmt) {
            case Format::Flt:
            case Format::Dec:
                *std::get_if<double>(&m_refs[i].val) = std::stod(m_match[i + 1]);
                break;
            case Format::Str:
                *std::get_if<std::string>(&m_refs[i].val) = m_match[i + 1];
                break;
        }
    }
}
