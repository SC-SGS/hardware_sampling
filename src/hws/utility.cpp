/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/utility.hpp"

#include <algorithm>    // std::min, std::transform, std::all_of
#include <cctype>       // std::tolower, std::isdigit
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

namespace hws::detail {

bool starts_with(const std::string_view sv, const std::string_view start) noexcept {
    return sv.substr(0, start.size()) == start;
}

std::string_view trim(std::string_view str) noexcept {
    // trim right
    {
        const std::string_view::size_type pos = std::min(str.find_last_not_of(" \t\v\r\n\f") + 1, str.size());
        str = str.substr(0, pos);
    }
    // trim left
    {
        const std::string_view::size_type pos = std::min(str.find_first_not_of(" \t\v\r\n\f"), str.size());
        str = str.substr(pos);
    }
    return str;
}

std::string to_lower_case(const std::string_view str) {
    std::string lowercase_str{ str };
    std::transform(str.begin(), str.end(), lowercase_str.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(static_cast<int>(c))); });
    return lowercase_str;
}

std::vector<std::string_view> split(const std::string_view str, const char delim) {
    std::vector<std::string_view> split_str;

    // if the input str is empty, return an empty vector
    if (str.empty()) {
        return split_str;
    }

    std::string_view::size_type pos = 0;
    std::string_view::size_type next = 0;
    while (next != std::string_view::npos) {
        next = str.find_first_of(delim, pos);
        split_str.emplace_back(next == std::string_view::npos ? str.substr(pos) : str.substr(pos, next - pos));
        pos = next + 1;
    }
    return split_str;
}

bool is_integer(std::string_view str) {
    return std::all_of(str.cbegin(), str.cend(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
}

}  // namespace hws::detail
