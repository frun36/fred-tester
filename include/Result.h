#pragma once

#include <expected>
#include <format>
#include <string>

template<typename T>
using Result = std::expected<T, std::string>;

template<typename... Args>
inline std::unexpected<std::string> Error(
    const std::format_string<Args...>& fmt,
    Args&&... args
) {
    return std::unexpected<std::string>(std::format(fmt, std::forward<Args>(args)...));
}
