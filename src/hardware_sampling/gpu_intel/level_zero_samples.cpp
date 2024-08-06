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

std::string level_zero_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device byte order
    if (this->byte_order_.has_value()) {
        str += std::format("  byte_order:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->byte_order_.value());
    }
    // the model name
    if (this->name_.has_value()) {
        str += std::format("  model_name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }
    // the standby mode
    if (this->standby_mode_.has_value()) {
        str += std::format("  standby_mode:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->standby_mode_.value());
    }
    // the number of threads per EU unit
    if (this->num_threads_per_eu_.has_value()) {
        str += std::format("  num_threads_per_eu:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_threads_per_eu_.value());
    }
    // the EU SIMD width
    if (this->eu_simd_width_.has_value()) {
        str += std::format("  physical_eu_simd_width:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->eu_simd_width_.value());
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_general_samples &samples) {
    return out << std::format("byte_order [string]: {}\n"
                              "name [string]: {}\n"
                              "standby_mode [string]: {}\n"
                              "num_threads_per_eu [int]: {}\n"
                              "eu_simd_width [int]: {}",
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_name()),
                              detail::value_or_default(samples.get_standby_mode()),
                              detail::value_or_default(samples.get_num_threads_per_eu()),
                              detail::value_or_default(samples.get_eu_simd_width()));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::string level_zero_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // minimum GPU core clock
    if (this->clock_gpu_min_.has_value()) {
        str += std::format("  clock_gpu_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_gpu_min_.value());
    }
    // maximum GPU core clock
    if (this->clock_gpu_max_.has_value()) {
        str += std::format("  clock_gpu_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_gpu_max_.value());
    }
    // all possible GPU core clock frequencies
    if (this->available_clocks_gpu_.has_value()) {
        str += std::format("  available_clocks_gpu:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->available_clocks_gpu_.value(), ", "));
    }
    // minimum memory clock
    if (this->clock_mem_min_.has_value()) {
        str += std::format("  clock_mem_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_mem_min_.value());
    }
    // maximum memory clock
    if (this->clock_mem_max_.has_value()) {
        str += std::format("  clock_mem_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_mem_max_.value());
    }
    // all possible memory clock frequencies
    if (this->available_clocks_mem_.has_value()) {
        str += std::format("  available_clocks_mem:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->available_clocks_mem_.value(), ", "));
    }

    // the maximum GPU core frequency based on the current TDP limit
    if (this->tdp_frequency_limit_gpu_.has_value()) {
        str += std::format("  tdp_frequency_limit_gpu:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->tdp_frequency_limit_gpu_.value(), ", "));
    }
    // the current GPU core clock frequency
    if (this->clock_gpu_.has_value()) {
        str += std::format("  clock_gpu:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_gpu_.value(), ", "));
    }
    // the current GPU core throttle reason
    if (this->throttle_reason_gpu_.has_value()) {
        str += std::format("  throttle_reason_gpu:\n"
                           "    unit: \"bitmask\"\n"
                           "    values: [{}]\n",
                           detail::join(this->throttle_reason_gpu_.value(), ", "));
    }
    // the maximum memory frequency based on the current TDP limit
    if (this->tdp_frequency_limit_mem_.has_value()) {
        str += std::format("  tdp_frequency_limit_mem:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->tdp_frequency_limit_mem_.value(), ", "));
    }
    // the current memory clock frequency
    if (this->clock_mem_.has_value()) {
        str += std::format("  clock_mem:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_mem_.value(), ", "));
    }
    // the current memory throttle reason
    if (this->throttle_reason_mem_.has_value()) {
        str += std::format("  throttle_reason_mem:\n"
                           "    unit: \"bitmask\"\n"
                           "    values: [{}]\n",
                           detail::join(this->throttle_reason_mem_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string level_zero_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // flag whether the energy threshold is enabled
    if (this->energy_threshold_enabled_.has_value()) {
        str += std::format("  energy_threshold_enabled:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->energy_threshold_enabled_.value());
    }
    // the energy threshold
    if (this->energy_threshold_.has_value()) {
        str += std::format("  energy_threshold:\n"
                           "    unit: \"J\"\n"
                           "    values: {}\n",
                           this->energy_threshold_.value());
    }

    // the total consumed energy
    if (this->power_total_energy_consumption_.has_value()) {
        decltype(level_zero_power_samples::power_total_energy_consumption_)::value_type consumed_energy(this->power_total_energy_consumption_->size());
        for (std::size_t i = 0; i < consumed_energy.size(); ++i) {
            consumed_energy[i] = this->power_total_energy_consumption_.value()[i] - this->power_total_energy_consumption_->front();
        }
        str += std::format("  power_total_energy_consumed:\n"
                           "    unit: \"J\"\n"
                           "    values: [{}]\n",
                           detail::join(consumed_energy, ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string level_zero_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // the total memory
    if (this->memory_total_.has_value()) {
        for (const auto &[key, value] : this->memory_total_.value()) {
            str += std::format("  memory_total_{}:\n"
                               "    unit: \"B\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the total allocatable memory
    if (this->allocatable_memory_total_.has_value()) {
        for (const auto &[key, value] : this->allocatable_memory_total_.value()) {
            str += std::format("  allocatable_memory_total_{}:\n"
                               "    unit: \"B\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the pcie max bandwidth
    if (this->pcie_link_max_speed_.has_value()) {
        str += std::format("  pcie_max_bandwidth:\n"
                           "    unit: \"BPS\"\n"
                           "    values: {}\n",
                           this->pcie_link_max_speed_.value());
    }
    // the pcie link width
    if (this->pcie_max_width_.has_value()) {
        str += std::format("  max_pcie_link_width:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->pcie_max_width_.value());
    }
    // the pcie generation
    if (this->max_pcie_link_generation_.has_value()) {
        str += std::format("  max_pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_pcie_link_generation_.value());
    }
    // the memory bus width
    if (this->bus_width_.has_value()) {
        for (const auto &[key, value] : this->bus_width_.value()) {
            str += std::format("  memory_bus_width_{}:\n"
                               "    unit: \"Bit\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the number of memory channels
    if (this->num_channels_.has_value()) {
        for (const auto &[key, value] : this->num_channels_.value()) {
            str += std::format("  memory_num_channels_{}:\n"
                               "    unit: \"int\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the memory location (system or device)
    if (this->location_.has_value()) {
        for (const auto &[key, value] : this->location_.value()) {
            str += std::format("  memory_location_{}:\n"
                               "    unit: \"string\"\n"
                               "    values: \"{}\"\n",
                               key,
                               value);
        }
    }

    // the currently free and used memory
    if (this->memory_free_.has_value()) {
        for (const auto &[key, value] : this->memory_free_.value()) {
            str += std::format("  memory_free_{}:\n"
                               "    unit: \"string\"\n"
                               "    values: [{}]\n",
                               key,
                               detail::join(value, ", "));

            // calculate the used memory
            if (this->allocatable_memory_total_.has_value()) {
                decltype(level_zero_memory_samples::memory_free_)::value_type::mapped_type memory_used(value.size(), this->allocatable_memory_total_->at(key));
                for (std::size_t i = 0; i < memory_used.size(); ++i) {
                    memory_used[i] -= value[i];
                }
                str += std::format("  memory_used_{}:\n"
                                   "    unit: \"string\"\n"
                                   "    values: [{}]\n",
                                   key,
                                   detail::join(memory_used, ", "));
            }
        }
    }
    // PCIe link speed
    if (this->pcie_link_speed_.has_value()) {
        str += std::format("  pcie_bandwidth:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_speed_.value(), ", "));
    }
    // PCIe link width
    if (this->pcie_link_width_.has_value()) {
        str += std::format("  pcie_link_width:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_width_.value(), ", "));
    }
    // PCIe link generation
    if (this->pcie_link_generation_.has_value()) {
        str += std::format("  pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_generation_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string level_zero_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // the maximum sensor temperature
    if (this->temperature_max_.has_value()) {
        for (const auto &[key, value] : this->temperature_max_.value()) {
            str += std::format("  temperature_{}_max:\n"
                               "    unit: \"°C\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }

    // the current PSU temperatures
    if (this->temperature_psu_.has_value()) {
        str += std::format("  temperature_psu:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_psu_.value(), ", "));
    }
    // the current sensor temperatures
    if (this->temperature_.has_value()) {
        for (const auto &[key, value] : this->temperature_.value()) {
            str += std::format("  temperature_{}:\n"
                               "    unit: \"°C\"\n"
                               "    values: [{}]\n",
                               key,
                               detail::join(value, ", "));
        }
    }

    // remove last newline
    str.pop_back();

    return str;
}

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
