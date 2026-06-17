/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Utility functions for the hardware sampling.
 */

#ifndef HWS_UTILITY_HPP_
#define HWS_UTILITY_HPP_
#pragma once

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <charconv>       // std::from_chars
#include <chrono>         // std::chrono::duration
#include <cmath>          // std::trunc
#include <cstddef>        // std::size_t
#include <optional>       // std::optional
#include <stdexcept>      // std::runtime_error
#include <string>         // std::string, std::stof, std::stod, std::stold
#include <string_view>    // std::string_view
#include <system_error>   // std::errc
#include <type_traits>    // std::is_same_v, std::is_floating_point_v, std::remove_cv_t, std::remove_reference_t, std::true_type, std::false_type
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include <mpi.h>  // MPI_Comm
#endif

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

/**
 * @brief Remove the topmost cv-qualifiers from type @p T.
 */
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

/**
 * @brief The case if the type @p T isn't a std::vector.
 * @tparam T the type to check
 */
template <typename T>
struct is_vector : std::false_type { };

/**
 * @brief The case if the type @p T is a std::vector.
 * @tparam T the type to check
 */
template <typename T>
struct is_vector<std::vector<T>> : std::true_type { };

/**
 * @brief Evaluates to `true` if @p T is a std::vector, otherwise `false`.
 * @tparam T the type to check
 */
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
 * @brief Check whether @p str is an integer
 * @param[in] str the string to check
 * @return `true` if @p str is an integer, `false` otherwise
 */
[[nodiscard]] bool is_integer(std::string_view str);

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
template <typename T>
[[nodiscard]] inline std::vector<std::string> quote(const std::vector<T> &values) {
    std::vector<std::string> quoted{};
    quoted.reserve(values.size());

    // quote all values
    for (const T &val : values) {
        quoted.push_back(fmt::format("\"{}\"", val));
    }

    return quoted;
}

/**
 * @brief Prefix all lines in a string with the given indentation.
 * @param[in] text the input text
 * @param[in] prefix the prefix (indentation) added to each line
 * @return the indented string
 */
[[nodiscard]] std::string indent_lines(const std::string &text, const std::string &prefix);

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

#if defined(HWS_MPI_SUPPORT_ENABLED)
/**
 * @brief Gather YAML strings from all MPI ranks and assemble them in rank order on rank 0.
 *
 * @param[in] local_yaml the local YAML string contribution
 * @param[in] communicator the MPI communicator
 *
 * @return concatenated YAML string on rank 0, empty string on all other ranks
 */
[[nodiscard]] std::string gather_yaml_strings_mpi(const std::string &local_yaml, MPI_Comm communicator);

/**
 * @brief The mode to use for MPI sampling.
 * per_rank: each rank creates hardware samplers for all devices visible to that rank
 * whole_node: if the same device is visible to more than one rank, only one of those ranks creates a hardware sampler for that device
 */
enum class mpi_sampling_mode {
    per_rank,
    whole_node
};

/**
 * @brief Information about a node-local MPI communicator for whole-node sampling.
 */
struct hostname_comm_info {
    MPI_Comm node_comm = MPI_COMM_NULL;
    int node_rank = 0;
    int node_size = 1;
};

/**
 * @brief Create a node-local MPI communicator for whole-node sampling based on node hostnames.
 * @param comm the parent MPI communicator to split into node-local communicators
 * @return the node-local MPI communicator information
 */
inline hostname_comm_info make_hostname_comm(MPI_Comm comm) {
    int world_rank = 0, world_size = 0;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    // Gather all hostnames
    char name[MPI_MAX_PROCESSOR_NAME];
    int name_len = 0;
    MPI_Get_processor_name(name, &name_len);

    std::vector<int> name_lengths(world_size);
    MPI_Allgather(&name_len, 1, MPI_INT, name_lengths.data(), 1, MPI_INT, comm);

    // Build displacements and total byte count
    std::vector<int> displs(world_size);
    int total = 0;
    for (int i = 0; i < world_size; ++i) {
        displs[i] = total;
        total += name_lengths[i];
    }

    std::vector<char> all_names(total);
    MPI_Allgatherv(name, name_len, MPI_CHAR, all_names.data(), name_lengths.data(), displs.data(), MPI_CHAR, comm);

    // Assign colors locally on every rank
    //
    // All ranks hold identical copies of all_names, name_lengths, and displs,
    // so they can each compute the same deterministic color map independently.

    std::unordered_map<std::string_view, int> host_to_color;
    host_to_color.reserve(world_size);
    std::vector<int> colors(world_size);
    int next_color = 0;
    for (int r = 0; r < world_size; ++r) {
        // get host name of rank r
        std::string_view host(&all_names[displs[r]], static_cast<std::size_t>(name_lengths[r]));

        // try to insert it into the host_to_color map
        auto [it, inserted] = host_to_color.emplace(host, next_color);

        // check if host was new, if yes, increment color
        if (inserted) {
            ++next_color;
        }
        // save color of current rank, either from newly created or existing entry
        colors[r] = it->second;
    }

    // Split communicator

    hostname_comm_info info{};
    MPI_Comm_split(comm, colors[world_rank], world_rank, &info.node_comm);
    MPI_Comm_rank(info.node_comm, &info.node_rank);
    MPI_Comm_size(info.node_comm, &info.node_size);
    return info;
}

/**
 * @brief Free a node-local MPI communicator for whole-node sampling.
 * @param info the node-local MPI communicator information to free
 */
inline void free_hostname_comm(hostname_comm_info &info) {
    if (info.node_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&info.node_comm);
    }
}

enum class device_backend_kind {
    nvidia,
    amd,
    intel
};

struct visible_gpu_device {
    device_backend_kind backend;
    int local_index;          // device index for that backend on this rank
    std::string physical_id;  // stable per-node identifier
};

    #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
/**
 * @brief creates a list of all visible NVIDIA GPU devices
 *
 * @return a vector of all visible NVIDIA GPU devices on the local node, each with its local index and physical ID
 */
[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_nvidia_devices();
    #endif

    #if defined(HWS_FOR_AMD_GPUS_ENABLED)
/**
 * @brief creates a list of all visible AMD GPU devices
 *
 * @return a vector of all visible AMD GPU devices on the local node, each with its local index and physical ID
 */
[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_amd_devices();
    #endif

    #if defined(HWS_FOR_INTEL_GPUS_ENABLED)
/**
 * @brief creates a list of all visible Intel GPU devices
 *
 * @return a vector of all visible Intel GPU devices on the local node, each with its local index and physical ID
 */
[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_intel_devices();
    #endif

/**
 * Computes for each MPI rank a list of devices that have to be sampled by this rank. Ensures that
 * each device is sampled by exactly one rank.
 *
 * @param local_devices a vector of visible_gpu_device for the local rank, each containing a local index and a physical ID
 * @param node_comm a node local MPI communicator
 * @return all device indices that have to be sampled by this rank
 */
inline std::vector<int> owned_local_indices_for_backend(const std::vector<visible_gpu_device> &local_devices, MPI_Comm node_comm) {
    int node_rank = 0, node_size = 0;
    MPI_Comm_rank(node_comm, &node_rank);
    MPI_Comm_size(node_comm, &node_size);

    // Pack physical IDs into a newline-separated string
    std::string packed;
    for (const auto &d : local_devices) {
        packed += d.physical_id;
        packed += '\n';
    }
    const int local_size = static_cast<int>(packed.size());

    // Allgather sizes
    std::vector<int> sizes(node_size);
    MPI_Allgather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, node_comm);

    // Displacements and total length
    std::vector<int> displs(node_size);
    int total = 0;
    for (int r = 0; r < node_size; ++r) {
        displs[r] = total;
        total += sizes[r];
    }

    // Allgatherv packed physical IDs
    std::vector<char> all_data(total);
    MPI_Allgatherv(packed.data(), local_size, MPI_CHAR, all_data.data(), sizes.data(), displs.data(), MPI_CHAR, node_comm);

    // Build owner map: physical_id -> first node_rank that reports it
    std::unordered_map<std::string, int> owner_rank_for_id;
    owner_rank_for_id.reserve(local_devices.size() * 2 + 1);

    for (int r = 0; r < node_size; ++r) {
        if (sizes[r] == 0) {
            continue;
        }

        const char *base = all_data.data() + displs[r];
        const int len = sizes[r];

        int line_start = 0;
        while (line_start < len) {
            int line_end = line_start;
            while (line_end < len && base[line_end] != '\n') {
                ++line_end;
            }
            if (line_end > line_start) {
                const std::string id(base + line_start, base + line_end);  // copy just this ID
                owner_rank_for_id.emplace(id, r);                          // first insertion wins
            }
            line_start = line_end + 1;
        }
    }

    // Decide which local indices we own: those whose physical_id is mapped to node_rank
    std::vector<int> owned_indices;
    owned_indices.reserve(local_devices.size());

    for (const auto &d : local_devices) {
        auto it = owner_rank_for_id.find(d.physical_id);
        if (it != owner_rank_for_id.end() && it->second == node_rank) {
            owned_indices.push_back(d.local_index);
        }
    }

    return owned_indices;
}

#endif

}  // namespace hws::detail

#endif  // HWS_UTILITY_HPP_
