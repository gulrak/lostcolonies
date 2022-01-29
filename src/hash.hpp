#pragma once

#include <cstdint>

namespace detail {
    constexpr uint64_t fnv1a(char const* str, size_t length)
    {
        return ((length ? fnv1a(str, length - 1) : 14695981039346656037ULL) ^ static_cast<unsigned char>(str[length])) * 1099511628211ULL;
    }
}

constexpr uint64_t operator""_h(const char* str, size_t length)
{
    return detail::fnv1a(str, length);
}

