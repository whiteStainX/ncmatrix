#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace utf8 {
namespace detail {
constexpr bool is_continuation(unsigned char byte) {
    return (byte & 0xC0U) == 0x80U;
}

constexpr char32_t min_value_for_length(std::size_t additional_bytes) {
    switch (additional_bytes) {
    case 0:
        return 0x0U;
    case 1:
        return 0x80U;
    case 2:
        return 0x800U;
    case 3:
        return 0x10000U;
    default:
        return 0x0U;
    }
}
} // namespace detail

inline std::vector<char32_t> decode(std::string_view input) {
    std::vector<char32_t> result;
    result.reserve(input.size());

    std::size_t i = 0;
    while (i < input.size()) {
        const unsigned char byte = static_cast<unsigned char>(input[i]);

        // Skip UTF-8 BOM if present at the beginning of the input.
        if (i == 0 && byte == 0xEFU && input.size() >= 3) {
            const unsigned char b1 = static_cast<unsigned char>(input[1]);
            const unsigned char b2 = static_cast<unsigned char>(input[2]);
            if (b1 == 0xBBU && b2 == 0xBFU) {
                i += 3;
                continue;
            }
        }

        char32_t codepoint = 0;
        std::size_t additional_bytes = 0;

        if (byte <= 0x7FU) {
            codepoint = byte;
        } else if ((byte & 0xE0U) == 0xC0U) {
            codepoint = static_cast<char32_t>(byte & 0x1FU);
            additional_bytes = 1;
        } else if ((byte & 0xF0U) == 0xE0U) {
            codepoint = static_cast<char32_t>(byte & 0x0FU);
            additional_bytes = 2;
        } else if ((byte & 0xF8U) == 0xF0U) {
            codepoint = static_cast<char32_t>(byte & 0x07U);
            additional_bytes = 3;
        } else {
            result.push_back(U'?');
            ++i;
            continue;
        }

        if (i + additional_bytes >= input.size()) {
            result.push_back(U'?');
            ++i;
            continue;
        }

        bool valid = true;
        for (std::size_t j = 0; j < additional_bytes; ++j) {
            const unsigned char continuation = static_cast<unsigned char>(input[i + 1 + j]);
            if (!detail::is_continuation(continuation)) {
                valid = false;
                i += j + 1;
                break;
            }
            codepoint = static_cast<char32_t>((codepoint << 6U) | (continuation & 0x3FU));
        }

        if (!valid) {
            result.push_back(U'?');
            continue;
        }

        const char32_t min_value = detail::min_value_for_length(additional_bytes);
        if (codepoint < min_value || codepoint > 0x10FFFFU || (codepoint >= 0xD800U && codepoint <= 0xDFFFU)) {
            result.push_back(U'?');
            i += additional_bytes + 1;
            continue;
        }

        result.push_back(codepoint);
        i += additional_bytes + 1;
    }

    return result;
}

} // namespace utf8

