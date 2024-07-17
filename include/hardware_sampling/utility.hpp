/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Utility functions for the hardware sampling.
 */

#ifndef HARDWARE_SAMPLING_UTILITY_HPP_
#define HARDWARE_SAMPLING_UTILITY_HPP_
#pragma once

#include <charconv>     // std::from_chars
#include <chrono>       // std::chrono::milliseconds
#include <cstddef>      // std::size_t
#include <format>       // std::format
#include <optional>     // std::optional
#include <stdexcept>    // std::runtime_error
#include <string>       // std::string, std::stof, std::stod, std::stold
#include <string_view>  // std::string_view
#include <type_traits>  // std::is_same_v, std::remove_cvref_t
#include <vector>       // std::vector

namespace hws::detail {

/**
 * @brief Defines a public optional getter with name `get_sample_name` and a private optional member with name `sample_name_`.
 */
#define HWS_SAMPLE_STRUCT_FIXED_MEMBER(sample_type, sample_name)                         \
  public:                                                                                \
    [[nodiscard]] const std::optional<sample_type> &get_##sample_name() const noexcept { \
        return sample_name##_;                                                           \
    }                                                                                    \
                                                                                         \
  private:                                                                               \
    std::optional<sample_type> sample_name##_{};

/**
 * @brief Defines a public optional vector getter with name `get_sample_name` and a private optional vector member with name `sample_name_`.
 * @details Same as `HWS_SAMPLE_STRUCT_FIXED_MEMBER` but per sample_name multiple values can be tracked.
 */
#define HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(sample_type, sample_name)                                   \
  public:                                                                                             \
    [[nodiscard]] const std::optional<std::vector<sample_type>> &get_##sample_name() const noexcept { \
        return sample_name##_;                                                                        \
    }                                                                                                 \
                                                                                                      \
  private:                                                                                            \
    std::optional<std::vector<sample_type>> sample_name##_{};

/**
 * @brief Convert all time points to their duration passed since the @p reference time point.
 * @tparam Duration the duration type to return
 * @tparam TimePoint the type if the time points
 * @param[in] time_points the time points
 * @param[in] reference the reference time point
 * @return the duration passed since the @p reference time point (`[[nodiscard]]`)
 */
template <typename Duration = std::chrono::milliseconds, typename TimePoint>
[[nodiscard]] inline std::vector<Duration> durations_from_reference_time(const std::vector<TimePoint> &time_points, const TimePoint &reference) {
    std::vector<Duration> durations(time_points.size());

    for (std::size_t i = 0; i < durations.size(); ++i) {
        durations[i] = std::chrono::duration_cast<Duration>(time_points[i] - reference);
    }

    return durations;
}

/**
 * @brief Convert all time points to their duration since the epoch start.
 * @tparam TimePoint the type of the time points
 * @param[in] time_points the time points
 * @return the duration passed since the respective @p TimePoint epoch start (`[[nodiscard]]`)
 */
template <typename TimePoint>
[[nodiscard]] inline std::vector<typename TimePoint::duration> time_points_to_epoch(const std::vector<TimePoint> &time_points) {
    std::vector<typename TimePoint::duration> times(time_points.size());

    for (std::size_t i = 0; i < times.size(); ++i) {
        times[i] = time_points[i].time_since_epoch();
    }
    return times;
}

/**
 * @brief Return the value encapsulated by the std::optional @p opt if it contains a value, otherwise a default constructed @p T is returned.
 * @tparam T the type of the value stored in the std::optional
 * @param[in] opt the std::optional to check
 * @return the value of the std::optional or a default constructed @p T (`[[nodiscard]]`)
 */
template <typename T>
[[nodiscard]] inline T value_or_default(const std::optional<T> &opt) {
    if (opt.has_value()) {
        return opt.value();
    } else {
        return T{};
    }
}

[[nodiscard]] std::string_view trim(std::string_view str) noexcept;

[[nodiscard]] std::string to_lower_case(std::string_view str);

template <typename T>
[[nodiscard]] inline T convert_to(const std::string_view str) {
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>) {
        // convert string_view to string
        return std::string{ trim(str) };
    } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, bool>) {
        const std::string lower_case_str = to_lower_case(trim(str));
        // the string true
        if (lower_case_str == "true") {
            return true;
        }
        // the string false
        if (lower_case_str == "false") {
            return false;
        }
        // convert a number to its "long long" value and convert it to a bool: 0 -> false, otherwise true
        return static_cast<bool>(convert_to<long long>(str));
    } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, char>) {
        const std::string_view trimmed = trim(str);
        // since we expect a character, after trimming the string must only contain exactly one character
        if (trimmed.size() != 1) {
            throw std::runtime_error{ std::format("Can't convert '{}' to a value of type char!", str) };
        }
        return trimmed.front();
    } else if constexpr (std::is_floating_point_v<std::remove_cvref_t<T>>) {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, float>) {
            return std::stof(std::string{ str });
        } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, double>) {
            return std::stod(std::string{ str });
        } else {
            return std::stold(std::string{ str });
        }
    } else {
        // remove leading whitespaces
        const std::string_view trimmed_str = trim(str);

        // convert string to value fo type T
        T val;
        auto res = std::from_chars(trimmed_str.data(), trimmed_str.data() + trimmed_str.size(), val);
        if (res.ec != std::errc{}) {
            throw std::runtime_error{ std::format("Can't convert '{}' to a value of type T!", str) };
        }
        return val;
    }
}

template <typename T>
[[nodiscard]] inline std::vector<T> split_as(const std::string_view str, const char delim = ' ') {
    std::vector<std::string> split_str;

    // if the input str is empty, return an empty vector
    if (str.empty()) {
        return split_str;
    }

    std::string_view::size_type pos = 0;
    std::string_view::size_type next = 0;
    while (next != std::string_view::npos) {
        next = str.find_first_of(delim, pos);
        split_str.emplace_back(convert_to<T>(next == std::string_view::npos ? str.substr(pos) : str.substr(pos, next - pos)));
        pos = next + 1;
    }
    return split_str;
}

[[nodiscard]] std::vector<std::string_view> split(std::string_view str, char delim = ' ');

template <typename CharT>
struct basic_ostream_formatter : std::formatter<std::basic_string_view<CharT>, CharT> {
    template <typename T, typename OutputIt>
    OutputIt format(const T &value, std::basic_format_context<OutputIt, CharT> &ctx) const {
        std::basic_stringstream<CharT> ss;
        ss << value;
        return std::formatter<std::basic_string_view<CharT>, CharT>::format(
            ss.view(), ctx);
    }
};

using ostream_formatter = basic_ostream_formatter<char>;

template <typename Container>
[[nodiscard]] inline std::string join(const Container &c, const std::string_view delim) {
    if (c.empty()) {
        return "";
    } else if (c.size() == 1) {
        return std::format("{}", *c.cbegin());
    } else {
        std::string out{};
        for (auto it = c.cbegin(); it != std::prev(c.cend()); it = std::next(it)) {
            std::format_to(std::back_inserter(out), "{}{}", *it, delim);
        }
        std::format_to(std::back_inserter(out), "{}", *std::prev(c.end()));
        return out;
    }
}

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_UTILITY_HPP_
