/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, remove_cvref_t}

#include <ostream>      // std::ostream
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <type_traits>  // std::remove_cvref_t, std::false_type, std::true_type
#include <vector>       // std::vector

namespace hws {

namespace {

template <typename MapType>
void append_map_values(std::string &str, const std::string_view entry_name, const MapType &map) {
    if (map.has_value()) {
        for (const auto &[key, value] : map.value()) {
            if constexpr (detail::is_vector_v<detail::remove_cvref_t<decltype(value)>>) {
                str += fmt::format("{}_{}: [{}]\n", entry_name, key, fmt::join(value, ", "));
            } else {
                str += fmt::format("{}_{}: {}\n", entry_name, key, value);
            }
        }
    }
}

}  // namespace

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string level_zero_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device byte order
    if (this->byte_order_.has_value()) {
        str += fmt::format("  byte_order:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->byte_order_.value());
    }
    // the vendor specific ID
    if (this->vendor_id_.has_value()) {
        str += fmt::format("  vendor_id:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->vendor_id_.value());
    }
    // device name
    if (this->name_.has_value()) {
        str += fmt::format("  name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }
    // GPU specific flags
    if (this->flags_.has_value()) {
        str += fmt::format("  flags:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(detail::quote(this->flags_.value()), ", "));
    }
    // the standby mode
    if (this->standby_mode_.has_value()) {
        str += fmt::format("  standby_mode:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->standby_mode_.value());
    }
    // the number of threads per EU unit
    if (this->num_threads_per_eu_.has_value()) {
        str += fmt::format("  num_threads_per_eu:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_threads_per_eu_.value());
    }
    // the EU SIMD width
    if (this->eu_simd_width_.has_value()) {
        str += fmt::format("  eu_simd_width:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->eu_simd_width_.value());
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_general_samples &samples) {
    return out << fmt::format("byte_order [string]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "flags [string]: [{}]\n"
                              "standby_mode [string]: {}\n"
                              "num_threads_per_eu [int]: {}\n"
                              "eu_simd_width [int]: {}",
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_vendor_id()),
                              detail::value_or_default(samples.get_name()),
                              fmt::join(detail::value_or_default(samples.get_flags()), ", "),
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
    if (this->clock_frequency_min_.has_value()) {
        str += fmt::format("  clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_min_.value());
    }
    // maximum GPU core clock
    if (this->clock_frequency_max_.has_value()) {
        str += fmt::format("  clock_gpu_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_max_.value());
    }
    // minimum memory clock
    if (this->memory_clock_frequency_min_.has_value()) {
        str += fmt::format("  memory_clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_min_.value());
    }
    // maximum memory clock
    if (this->memory_clock_frequency_max_.has_value()) {
        str += fmt::format("  memory_clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_max_.value());
    }
    // all possible GPU core clock frequencies
    if (this->available_clock_frequencies_.has_value()) {
        str += fmt::format("  available_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_clock_frequencies_.value(), ", "));
    }
    // all possible memory clock frequencies
    if (this->available_memory_clock_frequencies_.has_value()) {
        str += fmt::format("  available_memory_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_memory_clock_frequencies_.value(), ", "));
    }

    // the current GPU core clock frequency
    if (this->clock_frequency_.has_value()) {
        str += fmt::format("  clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->clock_frequency_.value(), ", "));
    }
    // the current memory clock frequency
    if (this->memory_clock_frequency_.has_value()) {
        str += fmt::format("  memory_clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_clock_frequency_.value(), ", "));
    }
    // the current GPU core throttle reason
    if (this->throttle_reason_.has_value()) {
        str += fmt::format("  throttle_reason:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->throttle_reason_.value(), ", "));
    }
    // the current memory throttle reason
    if (this->memory_throttle_reason_.has_value()) {
        str += fmt::format("  memory_throttle_reason:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_throttle_reason_.value(), ", "));
    }
    // the maximum GPU core frequency based on the current TDP limit
    if (this->frequency_limit_tdp_.has_value()) {
        str += fmt::format("  frequency_limit_tdp:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->frequency_limit_tdp_.value(), ", "));
    }
    // the maximum memory frequency based on the current TDP limit
    if (this->memory_frequency_limit_tdp_.has_value()) {
        str += fmt::format("  memory_frequency_limit_tdp:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_frequency_limit_tdp_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_clock_samples &samples) {
    return out << fmt::format("clock_frequency_min [MHz]: {}\n"
                              "clock_frequency_max [MHz]: {}\n"
                              "memory_clock_frequency_min [MHz]: {}\n"
                              "memory_clock_frequency_max [MHz]: {}\n"
                              "available_clock_frequencies [MHz]: [{}]\n"
                              "available_memory_clock_frequencies [MHz]: [{}]\n"
                              "clock_frequency [MHz]: [{}]\n"
                              "memory_clock_frequency [MHz]: [{}]\n"
                              "throttle_reason [string]: [{}]\n"
                              "memory_throttle_reason [string]: [{}]\n"
                              "frequency_limit_tdp [MHz]: [{}]\n"
                              "memory_frequency_limit_tdp [MHz]: [{}]",
                              detail::value_or_default(samples.get_clock_frequency_min()),
                              detail::value_or_default(samples.get_clock_frequency_max()),
                              detail::value_or_default(samples.get_memory_clock_frequency_min()),
                              detail::value_or_default(samples.get_memory_clock_frequency_max()),
                              fmt::join(detail::value_or_default(samples.get_available_clock_frequencies()), ", "),
                              fmt::join(detail::value_or_default(samples.get_available_memory_clock_frequencies()), ", "),
                              fmt::join(detail::value_or_default(samples.get_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_throttle_reason()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_throttle_reason()), ", "),
                              fmt::join(detail::value_or_default(samples.get_frequency_limit_tdp()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_frequency_limit_tdp()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::string level_zero_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // power enforced limit
    if (this->power_enforced_limit_.has_value()) {
        str += fmt::format("  power_enforced_limit:\n"
                           "    unit: \"W\"\n"
                           "    values: {}\n",
                           this->power_enforced_limit_.value());
    }
    // power measurement type
    if (this->power_measurement_type_.has_value()) {
        str += fmt::format("  power_measurement_type:\n"
                           "    unit: \"string\"\n"
                           "    values: {}\n",
                           this->power_measurement_type_.value());
    }
    // the power management mode
    if (this->power_management_mode_.has_value()) {
        str += fmt::format("  power_management_mode:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->power_management_mode_.value());
    }

    // the current power draw
    if (this->power_usage_.has_value()) {
        str += fmt::format("  power_usage:\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_usage_.value(), ", "));
    }
    // the total consumed energy
    if (this->power_total_energy_consumption_.has_value()) {
        str += fmt::format("  power_total_energy_consumption:\n"
                           "    unit: \"J\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_total_energy_consumption_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_power_samples &samples) {
    return out << fmt::format("power_enforced_limit [W]: {}\n"
                              "power_measurement_type [string]: {}\n"
                              "power_management_mode [bool]: {}\n"
                              "power_usage [W]: [{}]\n"
                              "power_total_energy_consumption [J]: [{}]",
                              detail::value_or_default(samples.get_power_enforced_limit()),
                              detail::value_or_default(samples.get_power_measurement_type()),
                              detail::value_or_default(samples.get_power_management_mode()),
                              fmt::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::string level_zero_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // the total memory
    if (this->memory_total_.has_value()) {
        for (const auto &[key, value] : this->memory_total_.value()) {
            str += fmt::format("  {}_memory_total:\n"
                               "    unit: \"B\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the total allocatable memory
    if (this->visible_memory_total_.has_value()) {
        for (const auto &[key, value] : this->visible_memory_total_.value()) {
            str += fmt::format("  {}_visible_memory_total:\n"
                               "    unit: \"B\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the memory location (system or device)
    if (this->memory_location_.has_value()) {
        for (const auto &[key, value] : this->memory_location_.value()) {
            str += fmt::format("  {}_memory_location:\n"
                               "    unit: \"string\"\n"
                               "    values: \"{}\"\n",
                               key,
                               value);
        }
    }
    // the pcie link width
    if (this->num_pcie_lanes_max_.has_value()) {
        str += fmt::format("  num_pcie_lanes_max:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_pcie_lanes_max_.value());
    }
    // the pcie generation
    if (this->pcie_link_generation_max_.has_value()) {
        str += fmt::format("  pcie_link_generation_max:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->pcie_link_generation_max_.value());
    }
    // the pcie max bandwidth
    if (this->pcie_link_speed_max_.has_value()) {
        str += fmt::format("  pcie_link_speed_max:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: {}\n",
                           this->pcie_link_speed_max_.value());
    }
    // the memory bus width
    if (this->memory_bus_width_.has_value()) {
        for (const auto &[key, value] : this->memory_bus_width_.value()) {
            str += fmt::format("  {}_memory_bus_width:\n"
                               "    unit: \"Bit\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }
    // the number of memory channels
    if (this->memory_num_channels_.has_value()) {
        for (const auto &[key, value] : this->memory_num_channels_.value()) {
            str += fmt::format("  {}_memory_num_channels:\n"
                               "    unit: \"int\"\n"
                               "    values: {}\n",
                               key,
                               value);
        }
    }

    // the currently free memory
    if (this->memory_free_.has_value()) {
        for (const auto &[key, value] : this->memory_free_.value()) {
            str += fmt::format("  {}_memory_free:\n"
                               "    unit: \"string\"\n"
                               "    values: [{}]\n",
                               key,
                               fmt::join(value, ", "));
        }
    }
    // the currently used memory
    if (this->memory_used_.has_value()) {
        for (const auto &[key, value] : this->memory_used_.value()) {
            str += fmt::format("  {}_memory_used:\n"
                               "    unit: \"string\"\n"
                               "    values: [{}]\n",
                               key,
                               fmt::join(value, ", "));
        }
    }
    // PCIe link width
    if (this->num_pcie_lanes_.has_value()) {
        str += fmt::format("  num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->num_pcie_lanes_.value(), ", "));
    }
    // PCIe link generation
    if (this->pcie_link_generation_.has_value()) {
        str += fmt::format("  pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_generation_.value(), ", "));
    }
    // PCIe link speed
    if (this->pcie_link_speed_.has_value()) {
        str += fmt::format("  pcie_link_speed:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_speed_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_memory_samples &samples) {
    std::string str{};

    append_map_values(str, "memory_total [B]", samples.get_memory_total());
    append_map_values(str, "visible_memory_total [B]", samples.get_visible_memory_total());
    append_map_values(str, "memory_location [string]", samples.get_memory_location());
    str += fmt::format("num_pcie_lanes_max [int]: {}\n"
                       "pcie_link_generation_max [int]: {}\n"
                       "pcie_link_speed_max [MBPS]: {}\n",
                       detail::value_or_default(samples.get_num_pcie_lanes_max()),
                       detail::value_or_default(samples.get_pcie_link_generation_max()),
                       detail::value_or_default(samples.get_pcie_link_speed_max()));
    append_map_values(str, "memory_bus_width [Bit]", samples.get_memory_bus_width());
    append_map_values(str, "memory_num_channels [int]", samples.get_memory_num_channels());

    append_map_values(str, "memory_free [string]", samples.get_memory_free());
    append_map_values(str, "memory_used [string]", samples.get_memory_used());
    str += fmt::format("num_pcie_lanes [int]: [{}]\n"
                       "pcie_link_generation [int]: [{}]\n"
                       "pcie_link_speed [MBPS]: [{}]",
                       fmt::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "),
                       fmt::join(detail::value_or_default(samples.get_pcie_link_generation()), ", "),
                       fmt::join(detail::value_or_default(samples.get_pcie_link_speed()), ", "));

    return out << str;
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::string level_zero_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // the number of fans
    if (this->num_fans_.has_value()) {
        str += fmt::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // the maximum fan speed in RPM
    if (this->fan_speed_max_.has_value()) {
        str += fmt::format("  fan_speed_max:\n"
                           "    unit: \"RPM\"\n"
                           "    values: {}\n",
                           this->fan_speed_max_.value());
    }
    // the maximum GPU temperature
    if (this->temperature_max_.has_value()) {
        str += fmt::format("  temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_max_.value());
    }
    // the maximum memory temperature
    if (this->memory_temperature_max_.has_value()) {
        str += fmt::format("  memory_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->memory_temperature_max_.value());
    }
    // the maximum global temperature
    if (this->global_temperature_max_.has_value()) {
        str += fmt::format("  global_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->global_temperature_max_.value());
    }

    // the current fan speed in percent
    if (this->fan_speed_percentage_.has_value()) {
        str += fmt::format("  fan_speed_percentage:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->fan_speed_percentage_.value(), ", "));
    }
    // the current GPU temperature
    if (this->temperature_.has_value()) {
        str += fmt::format("  temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_.value(), ", "));
    }
    // the current memory temperature
    if (this->memory_temperature_.has_value()) {
        str += fmt::format("  memory_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_temperature_.value(), ", "));
    }
    // the current global temperature
    if (this->global_temperature_.has_value()) {
        str += fmt::format("  global_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->global_temperature_.value(), ", "));
    }
    // the current PSU temperature
    if (this->psu_temperature_.has_value()) {
        str += fmt::format("  psu_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->psu_temperature_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const level_zero_temperature_samples &samples) {
    return out << fmt::format("num_fans [int]: {}\n"
                              "fan_speed_max [RPM]: {}\n"
                              "temperature_max [°C]: {}\n"
                              "memory_temperature_max [°C]: {}\n"
                              "global_temperature_max [°C]: {}\n"
                              "fan_speed_percentage [%]: [{}]\n"
                              "temperature [°C]: [{}]\n"
                              "memory_temperature [°C]: [{}]\n"
                              "global_temperature [°C]: [{}]\n"
                              "psu_temperature [°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_fan_speed_max()),
                              detail::value_or_default(samples.get_temperature_max()),
                              detail::value_or_default(samples.get_memory_temperature_max()),
                              detail::value_or_default(samples.get_global_temperature_max()),
                              fmt::join(detail::value_or_default(samples.get_fan_speed_percentage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_global_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_psu_temperature()), ", "));
}

}  // namespace hws
