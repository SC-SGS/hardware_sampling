/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include <format>       // std::format
#include <ostream>      // std::ostream
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <type_traits>  // std::remove_cvref_t, std::false_type, std::true_type
#include <vector>       // std::vector

namespace hws {

namespace detail {

template <typename T>
struct is_vector : std::false_type { };

template <typename T>
struct is_vector<std::vector<T>> : std::true_type { };

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

template <typename MapType>
void append_map_values(std::string &str, const std::string_view entry_name, const MapType &map) {
    if (map.has_value()) {
        for (const auto &[key, value] : map.value()) {
            if constexpr (is_vector_v<std::remove_cvref_t<decltype(value)>>) {
                str += std::format("{}_{}: [{}]\n", entry_name, key, detail::join(value, ", "));
            } else {
                str += std::format("{}_{}: {}\n", entry_name, key, value);
            }
        }
    }
}

}  // namespace detail

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const level_zero_general_samples &samples) {
    return out << std::format("name [string]: {}\n"
                              "standby_mode [string]: {}\n"
                              "num_threads_per_eu [int]: {}\n"
                              "eu_simd_width [int]: {}",
                              detail::value_or_default(samples.get_name()),
                              detail::value_or_default(samples.get_standby_mode()),
                              detail::value_or_default(samples.get_num_threads_per_eu()),
                              detail::value_or_default(samples.get_eu_simd_width()));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const level_zero_clock_samples &samples) {
    return out << std::format("clock_gpu_min [MHz]: {}\n"
                              "clock_gpu_max [MHz]: {}\n"
                              "available_clocks_gpu [MHz]: [{}]\n"
                              "clock_mem_min [MHz]: {}\n"
                              "clock_mem_max [MHz]: {}\n"
                              "available_clocks_mem [MHz]: [{}]\n"
                              "tdp_frequency_limit_gpu [MHz]: [{}]\n"
                              "clock_gpu [MHz]: [{}]\n"
                              "throttle_reason_gpu [bitmask]: [{}]\n"
                              "tdp_frequency_limit_mem [MHz]: [{}]\n"
                              "clock_mem [MHz]: [{}]\n"
                              "throttle_reason_mem [bitmask]: [{}]",
                              detail::value_or_default(samples.get_clock_gpu_min()),
                              detail::value_or_default(samples.get_clock_gpu_max()),
                              detail::join(detail::value_or_default(samples.get_available_clocks_gpu()), ", "),
                              detail::value_or_default(samples.get_clock_mem_min()),
                              detail::value_or_default(samples.get_clock_mem_max()),
                              detail::join(detail::value_or_default(samples.get_available_clocks_mem()), ", "),
                              detail::join(detail::value_or_default(samples.get_tdp_frequency_limit_gpu()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_gpu()), ", "),
                              detail::join(detail::value_or_default(samples.get_throttle_reason_gpu()), ", "),
                              detail::join(detail::value_or_default(samples.get_tdp_frequency_limit_mem()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_mem()), ", "),
                              detail::join(detail::value_or_default(samples.get_throttle_reason_mem()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const level_zero_power_samples &samples) {
    return out << std::format("energy_threshold_enabled [bool]: {}\n"
                              "energy_threshold [J]: {}\n"
                              "power_total_energy_consumption [J]: [{}]",
                              detail::value_or_default(samples.get_energy_threshold_enabled()),
                              detail::value_or_default(samples.get_energy_threshold()),
                              detail::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const level_zero_memory_samples &samples) {
    std::string str{};

    detail::append_map_values(str, "memory_total [B]", samples.get_memory_total());
    detail::append_map_values(str, "allocatable_memory_total [B]", samples.get_allocatable_memory_total());

    str += std::format("pcie_link_max_speed [BPS]: {}\n"
                       "pcie_max_width [int]: {}\n"
                       "max_pcie_link_generation [int]: {}\n",
                       detail::value_or_default(samples.get_pcie_link_max_speed()),
                       detail::value_or_default(samples.get_pcie_max_width()),
                       detail::value_or_default(samples.get_max_pcie_link_generation()));

    detail::append_map_values(str, "bus_width [Bit]", samples.get_bus_width());
    detail::append_map_values(str, "num_channels [int]", samples.get_num_channels());
    detail::append_map_values(str, "location [string]", samples.get_location());
    detail::append_map_values(str, "memory_free [string]", samples.get_memory_free());

    str += std::format("pcie_link_speed [MBPS]: [{}]\n"
                       "pcie_link_width [int]: [{}]\n"
                       "pcie_link_generation [int]: [{}]",
                       detail::join(detail::value_or_default(samples.get_pcie_link_speed()), ", "),
                       detail::join(detail::value_or_default(samples.get_pcie_link_width()), ", "),
                       detail::join(detail::value_or_default(samples.get_pcie_link_generation()), ", "));

    return out << str;
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const level_zero_temperature_samples &samples) {
    std::string str{};

    detail::append_map_values(str, "temperature_max [°C]", samples.get_temperature_max());

    str += std::format("temperature_psu [°C]: [{}]\n",
                       detail::join(detail::value_or_default(samples.get_temperature_psu()), ", "));

    detail::append_map_values(str, "temperature [°C]", samples.get_temperature());

    // remove last newline
    str.pop_back();

    return out << str;
}

}  // namespace hws
