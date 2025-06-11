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
    return std::unexpected<std::string>(
        std::format(fmt, std::forward<Args>(args)...)
    );
}

#define TRY(_expr) \
    { \
        auto _expected = _expr; \
        if (!_expected) { \
            return std::unexpected(std::move(_expected.error())); \
        } \
    }

#define TRY_S(_expr, _onfailure) \
    { \
        auto _expected = _expr; \
        if (!_expected) { \
            _onfailure(); \
            return std::unexpected(std::move(_expected.error())); \
        } \
    }

#define TRY_ASSIGN(_expr, _var) \
    { \
        auto _expected = _expr; \
        if (!_expected) { \
            return std::unexpected(std::move(_expected.error())); \
        } \
        _var = std::move(*_expected); \
    }

#define TRY_ASSIGN_S(_expr, _onfailure, _var) \
    { \
        auto _expected = _expr; \
        if (!_expected) { \
            _onfailure(); \
            return std::unexpected(std::move(_expected.error())); \
        } \
        _var = std::move(*_expected); \
    }
