#pragma once

#include <cmath>
#include <string>

#include "Result.h"

namespace utils {

static constexpr const char* FLT = R"([+-]?(?:\d+\.\d*|\.\d+|\d+))";
static constexpr const char* DEC = R"([+-]?\d+)";
static constexpr const char* HEX = R"([+-]?0[xX][0-9a-fA-F]+)";
static constexpr const char* STR = R"([^,\n]+)";

static inline std::string repeat(std::string str, std::string sep, size_t n) {
    if (n == 0)
        return "";

    size_t i = 0;

    std::string res;
    res.reserve(n * str.size() + (n - 1) * sep.size());
    res += str;
    i++;
    for (; i < n; i++) {
        res += sep;
        res += str;
    }

    return res;
}

static constexpr const char* TCM = "TCM0";
static constexpr const char* PM = "PMA0";

static inline Result<double> parseDouble(const std::string& s) {
    try {
        return std::stod(s);
    } catch (const std::exception& e) {
        return err("Failed to parse double '{}': {}", s, e.what());
    }
}

static inline Result<long> parseInt(const std::string& s) {
    try {
        return std::stol(s);
    } catch (const std::exception& e) {
        return err("Failed to parse int '{}': {}", s, e.what());
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

static inline std::string type(std::string boardName) {
    if (boardName == "TCM0") {
        return "TCM";
    } else if (boardName.substr(0, 2) == "PM") {
        return "PM";
    } else {
        throw std::runtime_error("Invalid boardName");
    }
}

static inline std::string topic(std::string boardName, std::string topicName) {
    std::string boardType = type(boardName);
    return std::format("FRED/{}/{}/{}", boardType, boardName, topicName);
}

} // namespace utils
