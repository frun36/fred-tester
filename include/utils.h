#pragma once

#include <cmath>
#include <string>
#include <string_view>

#include "Result.h"

namespace utils {

static constexpr std::string_view FLT = R"([+-]?(?:\d+\.\d*|\.\d+|\d+))";
static constexpr std::string_view DEC = R"([+-]?\d+)";
static constexpr std::string_view HEX = R"([+-]?0[xX][0-9a-fA-F]+)";
static constexpr std::string_view STR = R"([^,]+)";

static inline Result<double> parseDouble(const std::string& s) {
    try {
        return std::stod(s);
    } catch (const std::exception& e) {
        return Error("Failed to parse double: {}", e.what());
    }
}

static inline Result<long> parseInt(const std::string& s) {
    try {
        return std::stol(s);
    } catch (const std::exception& e) {
        return Error("Failed to parse int: {}", e.what());
    }
}

class Welford {
  private:
    size_t m_count = 0;
    double m_mean = 0;
    double m_m2 = 0;

  public:
    void tick(double x) {
        m_count++;
        // Welford's algorithm
        double delta = x - m_mean;
        m_mean += delta / m_count;
        m_m2 += delta * (x - m_mean);
    }

    void reset() {
        m_count = 0;
        m_mean = 0;
        m_m2 = 0;
    }

    double mean() const {
        return m_mean;
    }

    double stddev() const {
        return (m_count > 1) ? std::sqrt(m_m2 / (m_count - 1)) : 0.0;
    }
};

static inline std::string shorten(
    const std::string& input,
    size_t maxLen = 512,
    std::string newlineReplacement = "⏎",
    std::string ellipsis = "..."
) {
    std::string out;
    if (maxLen != 0 && input.size() > maxLen) {
        out = input.substr(0, maxLen);
        out += ellipsis;
    } else {
        out = input;
    }

    if (newlineReplacement != "\n") {
        size_t pos = 0;
        while ((pos = out.find('\n', pos)) != std::string::npos) {
            out.replace(pos, 1, newlineReplacement);
            pos += newlineReplacement.size();
        }
    }

    return out;
}

} // namespace utils
