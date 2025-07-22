#pragma once

#include <cmath>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

#include "Result.h"

namespace utils {

static constexpr const char* FLT = R"([+-]?(?:\d+\.\d*|\.\d+|\d+))";
static constexpr const char* DEC = R"([+-]?\d+)";
static constexpr const char* HEX = R"([+-]?(?:0[xX])?[0-9a-fA-F]+)";
static constexpr const char* STR = R"([^,\n]+)";

std::string repeat(std::string str, std::string sep, size_t n);

struct Board {
    enum class BoardName {
        TCM0,
        PMA0,
        PMA1,
        PMA2,
        PMA3,
        PMA4,
        PMA5,
        PMA6,
        PMA7,
        PMA8,
        PMA9,
        PMC0,
        PMC1,
        PMC2,
        PMC3,
        PMC4,
        PMC5,
        PMC6,
        PMC7,
        PMC8,
        PMC9
    } identity;

  public:
    static Result<Board> fromName(std::string str);
    std::string name() const;

    bool operator==(const Board& other) const;

    bool isTcm() const;
    bool isPm() const;

    std::string_view type() const;
};

static constexpr Board TCM0 = {Board::BoardName::TCM0};

Result<double> parseDouble(const std::string& s);

Result<long> parseInt(const std::string& s);

class Welford {
  private:
    size_t m_count = 0;
    double m_mean = 0;
    double m_m2 = 0;

  public:
    void tick(double x);
    void reset();
    double mean() const;
    double stddev() const;
};

std::string shorten(
    const std::string& input,
    size_t maxLen = 512,
    std::string newlineReplacement = "⏎",
    std::string ellipsis = "..."
);

std::string shortenLines(
    const std::string& input,
    size_t maxLen = 128,
    std::string ellipsis = "..."
);

std::string topic(Board board, std::string topicName);

struct Channel {
    Board board;
    uint32_t chIdx;

    bool operator==(const Channel& o) const;

    Channel() = default;

    Channel(Board board, uint32_t chIdx) : board(board), chIdx(chIdx) {}

    static Result<Channel> fromStr(std::string s);
    std::string toStr() const;
};

} // namespace utils
