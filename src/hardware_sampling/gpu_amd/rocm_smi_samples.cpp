/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default,}

#include "fmt/format.h"         // fmt::format
#include "fmt/ranges.h"         // fmt::join
#include "rocm_smi/rocm_smi.h"  // RSMI_MAX_FAN_SPEED

#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string rocm_smi_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device architecture
    if (this->architecture_.has_value()) {
        str += fmt::format("  architecture:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->architecture_.value());
    }
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

    // device compute utilization
    if (this->compute_utilization_.has_value()) {
        str += fmt::format("  compute_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->compute_utilization_.value(), ", "));
    }
    // device memory utilization
    if (this->memory_utilization_.has_value()) {
        str += fmt::format("  memory_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_utilization_.value(), ", "));
    }
    // performance state
    if (this->performance_level_.has_value()) {
        str += fmt::format("  performance_state:\n"
                           "    unit: \"int - see rsmi_dev_perf_level_t\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->performance_level_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_general_samples &samples) {
    return out << fmt::format("architecture [string]: {}\n"
                              "byte_order [string]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "compute_utilization [%]: [{}]\n"
                              "memory_utilization [%]: [{}]\n"
                              "performance_level [int]: [{}]",
                              detail::value_or_default(samples.get_architecture()),
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_vendor_id()),
                              detail::value_or_default(samples.get_name()),
                              fmt::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_performance_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::string rocm_smi_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // system clock min frequencies
    if (this->clock_frequency_min_.has_value()) {
        str += fmt::format("  clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_min_.value());
    }
    // system clock max frequencies
    if (this->clock_frequency_max_.has_value()) {
        str += fmt::format("  clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_max_.value());
    }
    // memory clock min frequencies
    if (this->memory_clock_frequency_min_.has_value()) {
        str += fmt::format("  memory_clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_min_.value());
    }
    // memory clock max frequencies
    if (this->memory_clock_frequency_max_.has_value()) {
        str += fmt::format("  memory_clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_max_.value());
    }
    // socket clock min frequencies
    if (this->socket_clock_frequency_min_.has_value()) {
        str += fmt::format("  socket_clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->socket_clock_frequency_min_.value());
    }
    // socket clock max frequencies
    if (this->socket_clock_frequency_max_.has_value()) {
        str += fmt::format("  socket_clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->socket_clock_frequency_max_.value());
    }
    // the available clock frequencies
    if (this->available_clock_frequencies_.has_value()) {
        str += fmt::format("  available_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_clock_frequencies_.value(), ", "));
    }
    // the available memory clock frequencies
    if (this->available_memory_clock_frequencies_.has_value()) {
        str += fmt::format("  available_memory_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_memory_clock_frequencies_.value(), ", "));
    }

    // system clock frequency
    if (this->clock_frequency_.has_value()) {
        str += fmt::format("  clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->clock_frequency_.value(), ", "));
    }
    // memory clock frequency
    if (this->memory_clock_frequency_.has_value()) {
        str += fmt::format("  memory_clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_clock_frequency_.value(), ", "));
    }
    // socket clock frequency
    if (this->socket_clock_frequency_.has_value()) {
        str += fmt::format("  socket_clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->socket_clock_frequency_.value(), ", "));
    }
    // overdrive level
    if (this->overdrive_level_.has_value()) {
        str += fmt::format("  overdrive_level:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->overdrive_level_.value(), ", "));
    }
    // memory overdrive level
    if (this->memory_overdrive_level_.has_value()) {
        str += fmt::format("  memory_overdrive_level:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_overdrive_level_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_clock_samples &samples) {
    return out << fmt::format("clock_frequency_min [MHz]: {}\n"
                              "clock_frequency_max [MHz]: {}\n"
                              "memory_clock_frequency_min [MHz]: {}\n"
                              "memory_clock_frequency_max [MHz]: {}\n"
                              "socket_clock_frequency_min [MHz]: {}\n"
                              "socket_clock_frequency_max [MHz]: {}\n"
                              "available_clock_frequencies [MHz]: [{}]\n"
                              "available_memory_clock_frequencies [MHz]: [{}]\n"
                              "clock_frequency [MHz]: [{}]\n"
                              "memory_clock_frequency [MHz]: [{}]\n"
                              "socket_clock_frequency [MHz]: [{}]\n"
                              "overdrive_level [%]: [{}]\n"
                              "memory_overdrive_level [%]: [{}]",
                              detail::value_or_default(samples.get_clock_frequency_min()),
                              detail::value_or_default(samples.get_clock_frequency_max()),
                              detail::value_or_default(samples.get_memory_clock_frequency_min()),
                              detail::value_or_default(samples.get_memory_clock_frequency_max()),
                              detail::value_or_default(samples.get_socket_clock_frequency_min()),
                              detail::value_or_default(samples.get_socket_clock_frequency_max()),
                              fmt::join(detail::value_or_default(samples.get_available_clock_frequencies()), ", "),
                              fmt::join(detail::value_or_default(samples.get_available_memory_clock_frequencies()), ", "),
                              fmt::join(detail::value_or_default(samples.get_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_socket_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_overdrive_level()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_overdrive_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::string rocm_smi_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // power management limit
    if (this->power_management_limit_.has_value()) {
        str += fmt::format("  power_management_limit:\n"
                           "    unit: \"W\"\n"
                           "    values: {}\n",
                           this->power_management_limit_.value());
    }
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
                           "    values: \"{}\"\n",
                           this->power_measurement_type_.value());
    }
    // available power levels
    if (this->available_power_profiles_.has_value()) {
        str += fmt::format("  available_power_profiles:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_power_profiles_.value(), ", "));
    }

    // current power usage
    if (this->power_usage_.has_value()) {
        str += fmt::format("  power_usage:\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_usage_.value(), ", "));
    }
    // total energy consumed
    if (this->power_total_energy_consumption_.has_value()) {
        str += fmt::format("  power_total_energy_consumed:\n"
                           "    unit: \"J\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_total_energy_consumption_.value(), ", "));
    }
    // current power level
    if (this->power_profile_.has_value()) {
        str += fmt::format("  power_profile:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_profile_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_power_samples &samples) {
    return out << fmt::format("power_management_limit [W]: {}\n"
                              "power_enforced_limit [W]: {}\n"
                              "power_measurement_type [string]: {}\n"
                              "available_power_profiles [string]: [{}]\n"
                              "power_usage [W]: [{}]\n"
                              "power_total_energy_consumption [J]: [{}]\n"
                              "power_profile [string]: [{}]",
                              detail::value_or_default(samples.get_power_management_limit()),
                              detail::value_or_default(samples.get_power_enforced_limit()),
                              detail::value_or_default(samples.get_power_measurement_type()),
                              fmt::join(detail::value_or_default(samples.get_available_power_profiles()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_profile()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::string rocm_smi_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // total memory
    if (this->memory_total_.has_value()) {
        str += fmt::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // total visible memory
    if (this->visible_memory_total_.has_value()) {
        str += fmt::format("  visible_memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->visible_memory_total_.value());
    }
    // min number of PCIe lanes
    if (this->min_num_pcie_lanes_.has_value()) {
        str += fmt::format("  min_num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->min_num_pcie_lanes_.value());
    }
    // max number of PCIe lanes
    if (this->max_num_pcie_lanes_.has_value()) {
        str += fmt::format("  max_num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_num_pcie_lanes_.value());
    }

    // used memory
    if (this->memory_used_.has_value()) {
        str += fmt::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_used_.value(), ", "));
    }
    // free memory
    if (this->memory_used_.has_value() && this->memory_total_.has_value()) {
        decltype(rocm_smi_memory_samples::memory_used_)::value_type memory_free(this->memory_used_->size(), this->memory_total_.value());
        for (std::size_t i = 0; i < memory_free.size(); ++i) {
            memory_free[i] -= this->memory_used_.value()[i];
        }
        str += fmt::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(memory_free, ", "));
    }

    // PCIe bandwidth
    if (this->pcie_transfer_rate_.has_value()) {
        str += fmt::format("  pcie_bandwidth:\n"
                           "    unit: \"T/s\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_transfer_rate_.value(), ", "));
    }
    // number of PCIe lanes
    if (this->num_pcie_lanes_.has_value()) {
        str += fmt::format("  pcie_num_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->num_pcie_lanes_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_memory_samples &samples) {
    return out << fmt::format("memory_total [B]: {}\n"
                              "visible_memory_total [B]: {}\n"
                              "min_num_pcie_lanes [int]: {}\n"
                              "max_num_pcie_lanes [int]: {}\n"
                              "memory_used [B]: [{}]\n"
                              "pcie_transfer_rate [T/s]: [{}]\n"
                              "num_pcie_lanes [int]: [{}]",
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_visible_memory_total()),
                              detail::value_or_default(samples.get_min_num_pcie_lanes()),
                              detail::value_or_default(samples.get_max_num_pcie_lanes()),
                              fmt::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_transfer_rate()), ", "),
                              fmt::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::string rocm_smi_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // number of fans (emulated)
    if (this->num_fans_.has_value()) {
        str += fmt::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // maximum fan speed
    if (this->max_fan_speed_.has_value()) {
        str += fmt::format("  max_fan_speed:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_fan_speed_.value());
    }
    // minimum GPU edge temperature
    if (this->temperature_edge_min_.has_value()) {
        str += fmt::format("  temperature_gpu_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_edge_min_.value());
    }
    // maximum GPU edge temperature
    if (this->temperature_edge_max_.has_value()) {
        str += fmt::format("  temperature_gpu_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_edge_max_.value());
    }
    // minimum GPU hotspot temperature
    if (this->temperature_hotspot_min_.has_value()) {
        str += fmt::format("  temperature_hotspot_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hotspot_min_.value());
    }
    // maximum GPU hotspot temperature
    if (this->temperature_hotspot_max_.has_value()) {
        str += fmt::format("  temperature_hotspot_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hotspot_max_.value());
    }
    // minimum GPU memory temperature
    if (this->temperature_memory_min_.has_value()) {
        str += fmt::format("  temperature_memory_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_memory_min_.value());
    }
    // maximum GPU memory temperature
    if (this->temperature_memory_max_.has_value()) {
        str += fmt::format("  temperature_memory_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_memory_max_.value());
    }
    // minimum GPU HBM 0 temperature
    if (this->temperature_hbm_0_min_.has_value()) {
        str += fmt::format("  temperature_hbm_0_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_0_min_.value());
    }
    // maximum GPU HBM 0 temperature
    if (this->temperature_hbm_0_max_.has_value()) {
        str += fmt::format("  temperature_hbm_0_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_0_max_.value());
    }
    // minimum GPU HBM 1 temperature
    if (this->temperature_hbm_1_min_.has_value()) {
        str += fmt::format("  temperature_hbm_1_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_1_min_.value());
    }
    // maximum GPU HBM 1 temperature
    if (this->temperature_hbm_1_max_.has_value()) {
        str += fmt::format("  temperature_hbm_1_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_1_max_.value());
    }
    // minimum GPU HBM 2 temperature
    if (this->temperature_hbm_2_min_.has_value()) {
        str += fmt::format("  temperature_hbm_2_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_2_min_.value());
    }
    // maximum GPU HBM 2 temperature
    if (this->temperature_hbm_2_max_.has_value()) {
        str += fmt::format("  temperature_hbm_2_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_2_max_.value());
    }
    // minimum GPU HBM 3 temperature
    if (this->temperature_hbm_3_min_.has_value()) {
        str += fmt::format("  temperature_hbm_3_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_3_min_.value());
    }
    // maximum GPU HBM 3 temperature
    if (this->temperature_hbm_3_max_.has_value()) {
        str += fmt::format("  temperature_hbm_3_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_3_max_.value());
    }

    // fan speed
    if (this->fan_speed_.has_value()) {
        std::vector<double> fan_speed_percent(this->fan_speed_->size());
        for (std::size_t i = 0; i < fan_speed_percent.size(); ++i) {
            fan_speed_percent[i] = static_cast<double>(this->fan_speed_.value()[i]) / static_cast<double>(RSMI_MAX_FAN_SPEED);
        }
        str += fmt::format("  fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(fan_speed_percent, ", "));
    }
    // GPU edge temperature
    if (this->temperature_edge_.has_value()) {
        str += fmt::format("  temperature_gpu:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_edge_.value(), ", "));
    }
    // GPU hotspot temperature
    if (this->temperature_hotspot_.has_value()) {
        str += fmt::format("  temperature_hotspot:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_hotspot_.value(), ", "));
    }
    // GPU memory temperature
    if (this->temperature_memory_.has_value()) {
        str += fmt::format("  temperature_memory:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_memory_.value(), ", "));
    }
    // GPU HBM 0 temperature
    if (this->temperature_hbm_0_.has_value()) {
        str += fmt::format("  temperature_hbm_0:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_hbm_0_.value(), ", "));
    }
    // GPU HBM 1 temperature
    if (this->temperature_hbm_1_.has_value()) {
        str += fmt::format("  temperature_hbm_1:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_hbm_1_.value(), ", "));
    }
    // GPU HBM 2 temperature
    if (this->temperature_hbm_2_.has_value()) {
        str += fmt::format("  temperature_hbm_2:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_hbm_2_.value(), ", "));
    }
    // GPU HBM 3 temperature
    if (this->temperature_hbm_3_.has_value()) {
        str += fmt::format("  temperature_hbm_3:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_hbm_3_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_temperature_samples &samples) {
    return out << fmt::format("num_fans [int]: {}\n"
                              "max_fan_speed [int]: {}\n"
                              "temperature_edge_min [m°C]: {}\n"
                              "temperature_edge_max [m°C]: {}\n"
                              "temperature_hotspot_min [m°C]: {}\n"
                              "temperature_hotspot_max [m°C]: {}\n"
                              "temperature_memory_min [m°C]: {}\n"
                              "temperature_memory_max [m°C]: {}\n"
                              "temperature_hbm_0_min [m°C]: {}\n"
                              "temperature_hbm_0_max [m°C]: {}\n"
                              "temperature_hbm_1_min [m°C]: {}\n"
                              "temperature_hbm_1_max [m°C]: {}\n"
                              "temperature_hbm_2_min [m°C]: {}\n"
                              "temperature_hbm_2_max [m°C]: {}\n"
                              "temperature_hbm_3_min [m°C]: {}\n"
                              "temperature_hbm_3_max [m°C]: {}\n"
                              "fan_speed [%]: [{}]\n"
                              "temperature_edge [m°C]: [{}]\n"
                              "temperature_hotspot [m°C]: [{}]\n"
                              "temperature_memory [m°C]: [{}]\n"
                              "temperature_hbm_0 [m°C]: [{}]\n"
                              "temperature_hbm_1 [m°C]: [{}]\n"
                              "temperature_hbm_2 [m°C]: [{}]\n"
                              "temperature_hbm_3 [m°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_max_fan_speed()),
                              detail::value_or_default(samples.get_temperature_edge_min()),
                              detail::value_or_default(samples.get_temperature_edge_max()),
                              detail::value_or_default(samples.get_temperature_hotspot_min()),
                              detail::value_or_default(samples.get_temperature_hotspot_max()),
                              detail::value_or_default(samples.get_temperature_memory_min()),
                              detail::value_or_default(samples.get_temperature_memory_max()),
                              detail::value_or_default(samples.get_temperature_hbm_0_min()),
                              detail::value_or_default(samples.get_temperature_hbm_0_max()),
                              detail::value_or_default(samples.get_temperature_hbm_1_min()),
                              detail::value_or_default(samples.get_temperature_hbm_1_max()),
                              detail::value_or_default(samples.get_temperature_hbm_2_min()),
                              detail::value_or_default(samples.get_temperature_hbm_2_max()),
                              detail::value_or_default(samples.get_temperature_hbm_3_min()),
                              detail::value_or_default(samples.get_temperature_hbm_3_max()),
                              fmt::join(detail::value_or_default(samples.get_fan_speed()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_edge()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_hotspot()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_memory()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_hbm_0()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_hbm_1()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_hbm_2()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_hbm_3()), ", "));
}

}  // namespace hws
