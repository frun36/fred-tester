#include "utils.h"

namespace utils {

std::string repeat(std::string str, std::string sep, size_t n) {
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

Result<Board> Board::fromName(std::string str) {
    if (str == "TCM0")
        return Board {BoardName::TCM0};
    if (str == "PMA0")
        return Board {BoardName::PMA0};
    if (str == "PMA1")
        return Board {BoardName::PMA1};
    if (str == "PMA2")
        return Board {BoardName::PMA2};
    if (str == "PMA3")
        return Board {BoardName::PMA3};
    if (str == "PMA4")
        return Board {BoardName::PMA4};
    if (str == "PMA5")
        return Board {BoardName::PMA5};
    if (str == "PMA6")
        return Board {BoardName::PMA6};
    if (str == "PMA7")
        return Board {BoardName::PMA7};
    if (str == "PMA8")
        return Board {BoardName::PMA8};
    if (str == "PMA9")
        return Board {BoardName::PMA9};
    if (str == "PMC0")
        return Board {BoardName::PMC0};
    if (str == "PMC1")
        return Board {BoardName::PMC1};
    if (str == "PMC2")
        return Board {BoardName::PMC2};
    if (str == "PMC3")
        return Board {BoardName::PMC3};
    if (str == "PMC4")
        return Board {BoardName::PMC4};
    if (str == "PMC5")
        return Board {BoardName::PMC5};
    if (str == "PMC6")
        return Board {BoardName::PMC6};
    if (str == "PMC7")
        return Board {BoardName::PMC7};
    if (str == "PMC8")
        return Board {BoardName::PMC8};
    if (str == "PMC9")
        return Board {BoardName::PMC9};
    if (str == "VIRTUAL_SC0")
        return Board {BoardName::VIRTUAL_SC0};
    if (str == "VIRTUAL_SC1")
        return Board {BoardName::VIRTUAL_SC1};
    return err("Invalid board name: {}", str);
}

std::string Board::name() const {
    switch (identity) {
        case BoardName::TCM0:
            return "TCM0";
        case BoardName::PMA0:
            return "PMA0";
        case BoardName::PMA1:
            return "PMA1";
        case BoardName::PMA2:
            return "PMA2";
        case BoardName::PMA3:
            return "PMA3";
        case BoardName::PMA4:
            return "PMA4";
        case BoardName::PMA5:
            return "PMA5";
        case BoardName::PMA6:
            return "PMA6";
        case BoardName::PMA7:
            return "PMA7";
        case BoardName::PMA8:
            return "PMA8";
        case BoardName::PMA9:
            return "PMA9";
        case BoardName::PMC0:
            return "PMC0";
        case BoardName::PMC1:
            return "PMC1";
        case BoardName::PMC2:
            return "PMC2";
        case BoardName::PMC3:
            return "PMC3";
        case BoardName::PMC4:
            return "PMC4";
        case BoardName::PMC5:
            return "PMC5";
        case BoardName::PMC6:
            return "PMC6";
        case BoardName::PMC7:
            return "PMC7";
        case BoardName::PMC8:
            return "PMC8";
        case BoardName::PMC9:
            return "PMC9";
        case BoardName::VIRTUAL_SC0:
            return "VIRTUAL_SC0";
        case BoardName::VIRTUAL_SC1:
            return "VIRTUAL_SC1";
        default:
            return "????";
    }
}

bool Board::operator==(const Board& other) const {
    return identity == other.identity;
}

bool Board::isTcm() const {
    return identity == BoardName::TCM0;
}

bool Board::isPm() const {
    return !isTcm();
}

std::string_view Board::type() const {
    return isTcm() ? "TCM" : "PM";
}

Result<double> parseDouble(const std::string& s) {
    try {
        return std::stod(s);
    } catch (const std::exception& e) {
        return err("Failed to parse double '{}': {}", s, e.what());
    }
}

Result<long> parseInt(const std::string& s) {
    try {
        return std::stol(s);
    } catch (const std::exception& e) {
        return err("Failed to parse int '{}': {}", s, e.what());
    }
}

void Welford::tick(double x) {
    m_count++;
    // Welford's algorithm
    double delta = x - m_mean;
    m_mean += delta / m_count;
    m_m2 += delta * (x - m_mean);
}

void Welford::reset() {
    m_count = 0;
    m_mean = 0;
    m_m2 = 0;
}

double Welford::mean() const {
    return m_mean;
}

double Welford::stddev() const {
    return (m_count > 1) ? std::sqrt(m_m2 / (m_count - 1)) : 0.0;
}

std::string shorten(
    const std::string& input,
    size_t maxLen,
    std::string newlineReplacement,
    std::string ellipsis
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

std::string shortenLines(
    const std::string& input,
    size_t maxLen,
    std::string ellipsis
) {
    auto view =
        input | std::views::split('\n') | std::views::transform([&](auto&& s) {
            auto truncated = s | std::views::take(maxLen);
            std::string res(truncated.begin(), truncated.end());
            if (std::ranges::distance(truncated) < std::ranges::distance(s)) {
                res += ellipsis;
            }
            return res;
        });

    std::string res;
    for (auto&& line : view) {
        res += line + '\n';
    }
    res.pop_back();

    return res;
}

std::string topic(Board board, std::string topicName) {
    return std::format("FRED/{}/{}/{}", board.type(), board.name(), topicName);
}

bool Channel::operator==(const Channel& o) const {
    return board == o.board && chIdx == o.chIdx;
}

Result<Channel> Channel::fromStr(std::string s) {
    Board board;
    TRY_ASSIGN(Board::fromName(s.substr(0, 4)), board);
    uint32_t chNumber;
    TRY_ASSIGN(parseInt(s.substr(6, 2)), chNumber);
    if (chNumber < 1 || chNumber > 12) {
        return err("Invalid channel number {}", chNumber);
    }

    return Channel {board, chNumber - 1};
}

std::string Channel::toStr() const {
    return std::format("{}CH{:02}", board.name(), chIdx + 1);
}
} // namespace utils
