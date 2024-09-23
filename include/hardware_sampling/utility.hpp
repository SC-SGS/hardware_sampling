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

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "ryml.hpp"      // ryml::NodeRef, ryml::key, ryml::csubstr, ryml::MAP, ryml::SEQ, ryml::FLOW_SL, ryml::VAL_SQUO
#include "ryml_std.hpp"  // standard library interoperability

#include <charconv>      // std::from_chars
#include <chrono>        // std::chrono::duration
#include <cmath>         // std::trunc
#include <cstddef>       // std::size_t
#include <optional>      // std::optional
#include <stdexcept>     // std::runtime_error
#include <string>        // std::string, std::stof, std::stod, std::stold
#include <string_view>   // std::string_view
#include <system_error>  // std::errc
#include <type_traits>   // std::is_same_v, std::is_floating_point_v, std::remove_cv_t, std::remove_reference_t, std::true_type, std::false_type
#include <vector>        // std::vector

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

/*****************************************************************************************************/
/**                                          type_traits                                            **/
/*****************************************************************************************************/

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
struct is_vector : std::false_type { };

template <typename T>
struct is_vector<std::vector<T>> : std::true_type { };

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

/*****************************************************************************************************/
/**                                      string manipulation                                        **/
/*****************************************************************************************************/

/**
 * @brief Checks whether the string @p sv starts with the substring @p start
 * @param[in] sv the full string
 * @param[in] start the substring
 * @return `true` if @p sv starts with @p start, otherwise `false`
 */
[[nodiscard]] bool starts_with(std::string_view sv, std::string_view start) noexcept;

/**
 * @brief Trim the @p str, i.e., remove all leading and trailing whitespace characters.
 * @param[in] str the string to trim
 * @return the trimmed string (`[[nodiscard]]`)
 */
[[nodiscard]] std::string_view trim(std::string_view str) noexcept;

/**
 * @brief Convert the @p str to its lower case representation.
 * @param[in] str the string to convert to lower case
 * @return the lower case string (`[[nodiscard]]`)
 */
[[nodiscard]] std::string to_lower_case(std::string_view str);

/**
 * @brief Split the @p str at the delimiters @p delim.
 * @param[in] str the string to split
 * @param[in] delim the used delimiter
 * @return a vector containing all split tokens (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<std::string_view> split(std::string_view str, char delim = ' ');

/**
 * @brief Convert the @p str to a value of type @p T.
 * @tparam T the type to convert the string to
 * @param[in] str the string to convert
 * @return the value of type @p T (`[[nodiscard]]`)
 */
template <typename T>
[[nodiscard]] inline T convert_to(const std::string_view str) {
    if constexpr (std::is_same_v<detail::remove_cvref_t<T>, std::string>) {
        // convert string_view to string
        return std::string{ trim(str) };
    } else if constexpr (std::is_same_v<detail::remove_cvref_t<T>, bool>) {
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
    } else if constexpr (std::is_same_v<detail::remove_cvref_t<T>, char>) {
        const std::string_view trimmed = trim(str);
        // since we expect a character, after trimming the string must only contain exactly one character
        if (trimmed.size() != 1) {
            throw std::runtime_error{ fmt::format("Can't convert '{}' to a value of type char!", str) };
        }
        return trimmed.front();
    } else if constexpr (std::is_floating_point_v<detail::remove_cvref_t<T>>) {
        if constexpr (std::is_same_v<detail::remove_cvref_t<T>, float>) {
            return std::stof(std::string{ str });
        } else if constexpr (std::is_same_v<detail::remove_cvref_t<T>, double>) {
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
            throw std::runtime_error{ fmt::format("Can't convert '{}' to a value of type T!", str) };
        }
        return val;
    }
}

/**
 * @brief Split the @p str at the delimiters @p delim and convert each token to a value of type @p T.
 * @tparam T the type to convert the tokens to
 * @param[in] str the string to split
 * @param[in] delim the used delimiter
 * @return a vector containing all split tokens convert to a value of type T (`[[nodiscard]]`)
 */
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

/**
 * @brief Convert all entries in the map to a single dict-like string.
 * @details The resulting string is of form "{KEY, VALUE}" or "{KEY, [VALUES]}".
 * @tparam MapType the type of the map
 * @param[in] map the map to convert to a string
 * @return the result string (`[[nodiscard]]`(
 */
template <typename MapType>
[[nodiscard]] inline std::string map_entry_to_string(const std::optional<MapType> &map) {
    if (map.has_value()) {
        std::vector<std::string> entries{};
        for (const auto &[key, value] : map.value()) {
            if constexpr (is_vector_v<detail::remove_cvref_t<decltype(value)>>) {
                entries.push_back(fmt::format("{{{}, [{}]}}", key, fmt::join(value, ", ")));
            } else {
                entries.push_back(fmt::format("{{{}, {}}}", key, value));
            }
        }
        return fmt::format("{}", fmt::join(entries, ", "));
    }
    return "";
}

/**
 * @brief Quote all @p values and return a vector of strings.
 * @details Example: calling this function with `{ 1, 2, 3, 4 }` would return a vector of strings containing `{ "1", "2", "3", "4" }`.
 * @tparam T the type of the values to quote
 * @param[in] values the values to quote
 * @return the quoted values (`[[nodiscard]]`)
 */
//template <typename T>
//[[nodiscard]] inline std::vector<std::string> quote(const std::vector<T> &values) {
//    std::vector<std::string> quoted{};
//    quoted.reserve(values.size());
//
//    // quote all values
//    for (const T &val : values) {
//        quoted.push_back(fmt::format("\"{}\"", val));
//    }
//
//    return quoted;
//}

/*****************************************************************************************************/
/**                                        YAML manipulation                                        **/
/*****************************************************************************************************/

/**
 * @brief Add a new YAML entry to the @p node.
 * @tparam T the type of the YAML entry
 * @param[in,out] node the node to add this entry to
 * @param[in] entry_name the name of the entry
 * @param[in] entry_unit the unit of the entry
 * @param[in] entry_value the value of the entry
 * @param[in] turbostat_name optional: for turbostat entries the original turbostat name
 */
template <typename T>
void add_yaml_entry(ryml::NodeRef &node, const ryml::csubstr entry_name, const ryml::csubstr entry_unit, const std::optional<T> &entry_value, std::optional<ryml::csubstr> turbostat_name = std::nullopt) {
    if (entry_value.has_value()) {
        ryml::NodeRef entry = node.append_child() << ryml::key(entry_name);
        entry |= ryml::MAP;

        // if a turbostat name is provided, set it
        if (turbostat_name.has_value()) {
            entry.append_child() << ryml::key("turbostat_name") << turbostat_name.value();
        }

        // set the unit
        entry.append_child() << ryml::key("unit") << entry_unit;

        // set values based on whether it's a vector or not
        if constexpr (is_vector_v<T>) {
            ryml::NodeRef values = entry["values"];
            values |= ryml::SEQ | ryml::FLOW_SL;
            values << entry_value.value();
        } else if constexpr (std::is_same_v<T, std::string>) {
            ryml::NodeRef values = entry["values"];
            values |= ryml::VAL_SQUO;
            values << entry_value.value();
        } else if constexpr (std::is_same_v<T, bool>) {
            entry.append_child() << ryml::key("values") << fmt::format("{}", entry_value.value());
        } else {
            entry.append_child() << ryml::key("values") << entry_value.value();
        }
    }
}

/*****************************************************************************************************/
/**                                      other free functions                                       **/
/*****************************************************************************************************/

/**
 * @brief Convert the time point to its duration in seconds (using double) truncated to three decimal places passed since the @p reference time point.
 * @tparam TimePoint the type if the time point
 * @param[in] time_point the time point
 * @param[in] reference the reference time point
 * @return the duration passed in seconds since the @p reference time point (`[[nodiscard]]`)
 */
template <typename TimePoint>
[[nodiscard]] inline double duration_from_reference_time(const TimePoint &time_point, const TimePoint &reference) {
    return std::trunc(std::chrono::duration<double>(time_point - reference).count() * 1000.0) / 1000.0;
}

/**
 * @brief Convert all time points to their duration in seconds (using double) truncated to three decimal places passed since the @p reference time point.
 * @tparam TimePoint the type if the time points
 * @param[in] time_points the time points
 * @param[in] reference the reference time point
 * @return the duration passed in seconds since the @p reference time point (`[[nodiscard]]`)
 */
template <typename TimePoint>
[[nodiscard]] inline std::vector<double> durations_from_reference_time(const std::vector<TimePoint> &time_points, const TimePoint &reference) {
    std::vector<double> durations(time_points.size());

    for (std::size_t i = 0; i < durations.size(); ++i) {
        durations[i] = duration_from_reference_time(time_points[i], reference);
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

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_UTILITY_HPP_
