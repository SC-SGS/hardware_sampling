/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include "rocm_smi/rocm_smi.h"  // RSMI_MAX_FAN_SPEED

#include <format>   // std::format
#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string rocm_smi_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device byte order
    if (this->byte_order_.has_value()) {
        str += std::format("  byte_order:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->byte_order_.value());
    }
    // the vendor specific ID
    if (this->vendor_id_.has_value()) {
        str += std::format("  vendor_id:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->vendor_id_.value());
    }
    // device name
    if (this->name_.has_value()) {
        str += std::format("  name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }

    // device compute utilization
    if (this->compute_utilization_.has_value()) {
        str += std::format("  compute_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->compute_utilization_.value(), ", "));
    }
    // device memory utilization
    if (this->memory_utilization_.has_value()) {
        str += std::format("  memory_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_utilization_.value(), ", "));
    }
    // performance state
    if (this->performance_level_.has_value()) {
        str += std::format("  performance_state:\n"
                           "    unit: \"int - see rsmi_dev_perf_level_t\"\n"
                           "    values: [{}]\n",
                           detail::join(this->performance_level_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_general_samples &samples) {
    return out << std::format("byte_order [string]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "compute_utilization [%]: [{}]\n"
                              "memory_utilization [%]: [{}]\n"
                              "performance_level [int]: [{}]",
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_vendor_id()),
                              detail::value_or_default(samples.get_name()),
                              detail::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
                              detail::join(detail::value_or_default(samples.get_memory_utilization()), ", "),
                              detail::join(detail::value_or_default(samples.get_performance_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::string rocm_smi_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // socket clock min frequencies
    if (this->clock_socket_min_.has_value()) {
        str += std::format("  clock_socket_min:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_socket_min_.value());
    }
    // socket clock max frequencies
    if (this->clock_socket_max_.has_value()) {
        str += std::format("  clock_socket_max:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_socket_max_.value());
    }

    // memory clock min frequencies
    if (this->clock_memory_min_.has_value()) {
        str += std::format("  clock_memory_min:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_memory_min_.value());
    }
    // memory clock max frequencies
    if (this->clock_memory_max_.has_value()) {
        str += std::format("  clock_memory_max:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_memory_max_.value());
    }

    // system clock min frequencies
    if (this->clock_system_min_.has_value()) {
        str += std::format("  clock_gpu_min:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_system_min_.value());
    }
    // system clock max frequencies
    if (this->clock_system_max_.has_value()) {
        str += std::format("  clock_gpu_max:\n"
                           "    unit: \"Hz\"\n"
                           "    values: {}\n",
                           this->clock_system_max_.value());
    }

    // socket clock frequency
    if (this->clock_socket_.has_value()) {
        str += std::format("  clock_socket:\n"
                           "    unit: \"Hz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_socket_.value(), ", "));
    }
    // memory clock frequency
    if (this->clock_memory_.has_value()) {
        str += std::format("  clock_memory:\n"
                           "    unit: \"Hz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_memory_.value(), ", "));
    }
    // system clock frequency
    if (this->clock_system_.has_value()) {
        str += std::format("  clock_gpu:\n"
                           "    unit: \"Hz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_system_.value(), ", "));
    }
    // overdrive level
    if (this->overdrive_level_.has_value()) {
        str += std::format("  overdrive_level:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->overdrive_level_.value(), ", "));
    }
    // memory overdrive level
    if (this->memory_overdrive_level_.has_value()) {
        str += std::format("  memory_overdrive_level:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_overdrive_level_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_clock_samples &samples) {
    return out << std::format("clock_system_min [Hz]: {}\n"
                              "clock_system_max [Hz]: {}\n"
                              "clock_socket_min [Hz]: {}\n"
                              "clock_socket_max [Hz]: {}\n"
                              "clock_memory_min [Hz]: {}\n"
                              "clock_memory_max [Hz]: {}\n"
                              "clock_system [Hz]: [{}]\n"
                              "clock_socket [Hz]: [{}]\n"
                              "clock_memory [Hz]: [{}]\n"
                              "overdrive_level [%]: [{}]\n"
                              "memory_overdrive_level [%]: [{}]",
                              detail::value_or_default(samples.get_clock_system_min()),
                              detail::value_or_default(samples.get_clock_system_max()),
                              detail::value_or_default(samples.get_clock_socket_min()),
                              detail::value_or_default(samples.get_clock_socket_max()),
                              detail::value_or_default(samples.get_clock_memory_min()),
                              detail::value_or_default(samples.get_clock_memory_max()),
                              detail::join(detail::value_or_default(samples.get_clock_system()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_socket()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_memory()), ", "),
                              detail::join(detail::value_or_default(samples.get_overdrive_level()), ", "),
                              detail::join(detail::value_or_default(samples.get_memory_overdrive_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::string rocm_smi_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // default power cap
    if (this->power_default_cap_.has_value()) {
        str += std::format("  power_management_limit:\n"
                           "    unit: \"muW\"\n"
                           "    values: {}\n",
                           this->power_default_cap_.value());
    }
    // power cap
    if (this->power_cap_.has_value()) {
        str += std::format("  power_enforced_limit:\n"
                           "    unit: \"muW\"\n"
                           "    values: {}\n",
                           this->power_cap_.value());
    }
    // power measurement type
    if (this->power_type_.has_value()) {
        str += std::format("  power_measurement_type:\n"
                           "    unit: \"string\"\n"
                           "    values: {}\n",
                           this->power_type_.value());
    }
    // available power levels
    if (this->available_power_profiles_.has_value()) {
        str += std::format("  available_power_profiles:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           detail::join(this->available_power_profiles_.value(), ", "));
    }

    // current power usage
    if (this->power_usage_.has_value()) {
        str += std::format("  power_usage:\n"
                           "    unit: \"muW\"\n"
                           "    values: [{}]\n",
                           detail::join(this->power_usage_.value(), ", "));
    }
    // total energy consumed
    if (this->power_total_energy_consumption_.has_value()) {
        decltype(rocm_smi_power_samples::power_total_energy_consumption_)::value_type consumed_energy(this->power_total_energy_consumption_->size());
        for (std::size_t i = 0; i < consumed_energy.size(); ++i) {
            consumed_energy[i] = this->power_total_energy_consumption_.value()[i] - this->power_total_energy_consumption_->front();
        }
        str += std::format("  power_total_energy_consumed:\n"
                           "    unit: \"muJ\"\n"
                           "    values: [{}]\n",
                           detail::join(consumed_energy, ", "));
    }
    // current power level
    if (this->power_profile_.has_value()) {
        str += std::format("  power_profile:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           detail::join(this->power_profile_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_power_samples &samples) {
    return out << std::format("power_default_cap [muW]: {}\n"
                              "power_cap [muW]: {}\n"
                              "power_type [string]: {}\n"
                              "available_power_profiles [string]: [{}]\n"
                              "power_usage [muW]: [{}]\n"
                              "power_total_energy_consumption [muJ]: [{}]\n"
                              "power_profile [string]: [{}]",
                              detail::value_or_default(samples.get_power_default_cap()),
                              detail::value_or_default(samples.get_power_cap()),
                              detail::value_or_default(samples.get_power_type()),
                              detail::join(detail::value_or_default(samples.get_available_power_profiles()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_profile()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::string rocm_smi_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // total memory
    if (this->memory_total_.has_value()) {
        str += std::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // total visible memory
    if (this->visible_memory_total_.has_value()) {
        str += std::format("  visible_memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->visible_memory_total_.value());
    }
    // min number of PCIe lanes
    if (this->min_num_pcie_lanes_.has_value()) {
        str += std::format("  min_num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->min_num_pcie_lanes_.value());
    }
    // max number of PCIe lanes
    if (this->max_num_pcie_lanes_.has_value()) {
        str += std::format("  max_num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_num_pcie_lanes_.value());
    }

    // used memory
    if (this->memory_used_.has_value()) {
        str += std::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_used_.value(), ", "));
    }
    // free memory
    if (this->memory_used_.has_value() && this->memory_total_.has_value()) {
        decltype(rocm_smi_memory_samples::memory_used_)::value_type memory_free(this->memory_used_->size(), this->memory_total_.value());
        for (std::size_t i = 0; i < memory_free.size(); ++i) {
            memory_free[i] -= this->memory_used_.value()[i];
        }
        str += std::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(memory_free, ", "));
    }

    // PCIe bandwidth
    if (this->pcie_transfer_rate_.has_value()) {
        str += std::format("  pcie_bandwidth:\n"
                           "    unit: \"T/s\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_transfer_rate_.value(), ", "));
    }
    // number of PCIe lanes
    if (this->num_pcie_lanes_.has_value()) {
        str += std::format("  pcie_num_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->num_pcie_lanes_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_memory_samples &samples) {
    return out << std::format("memory_total [B]: {}\n"
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
                              detail::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              detail::join(detail::value_or_default(samples.get_pcie_transfer_rate()), ", "),
                              detail::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::string rocm_smi_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // number of fans (emulated)
    if (this->num_fans_.has_value()) {
        str += std::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // maximum fan speed
    if (this->max_fan_speed_.has_value()) {
        str += std::format("  max_fan_speed:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_fan_speed_.value());
    }
    // minimum GPU edge temperature
    if (this->temperature_edge_min_.has_value()) {
        str += std::format("  temperature_gpu_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_edge_min_.value());
    }
    // maximum GPU edge temperature
    if (this->temperature_edge_max_.has_value()) {
        str += std::format("  temperature_gpu_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_edge_max_.value());
    }
    // minimum GPU hotspot temperature
    if (this->temperature_hotspot_min_.has_value()) {
        str += std::format("  temperature_hotspot_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hotspot_min_.value());
    }
    // maximum GPU hotspot temperature
    if (this->temperature_hotspot_max_.has_value()) {
        str += std::format("  temperature_hotspot_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hotspot_max_.value());
    }
    // minimum GPU memory temperature
    if (this->temperature_memory_min_.has_value()) {
        str += std::format("  temperature_memory_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_memory_min_.value());
    }
    // maximum GPU memory temperature
    if (this->temperature_memory_max_.has_value()) {
        str += std::format("  temperature_memory_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_memory_max_.value());
    }
    // minimum GPU HBM 0 temperature
    if (this->temperature_hbm_0_min_.has_value()) {
        str += std::format("  temperature_hbm_0_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_0_min_.value());
    }
    // maximum GPU HBM 0 temperature
    if (this->temperature_hbm_0_max_.has_value()) {
        str += std::format("  temperature_hbm_0_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_0_max_.value());
    }
    // minimum GPU HBM 1 temperature
    if (this->temperature_hbm_1_min_.has_value()) {
        str += std::format("  temperature_hbm_1_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_1_min_.value());
    }
    // maximum GPU HBM 1 temperature
    if (this->temperature_hbm_1_max_.has_value()) {
        str += std::format("  temperature_hbm_1_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_1_max_.value());
    }
    // minimum GPU HBM 2 temperature
    if (this->temperature_hbm_2_min_.has_value()) {
        str += std::format("  temperature_hbm_2_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_2_min_.value());
    }
    // maximum GPU HBM 2 temperature
    if (this->temperature_hbm_2_max_.has_value()) {
        str += std::format("  temperature_hbm_2_max:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_2_max_.value());
    }
    // minimum GPU HBM 3 temperature
    if (this->temperature_hbm_3_min_.has_value()) {
        str += std::format("  temperature_hbm_3_min:\n"
                           "    unit: \"m°C\"\n"
                           "    values: {}\n",
                           this->temperature_hbm_3_min_.value());
    }
    // maximum GPU HBM 3 temperature
    if (this->temperature_hbm_3_max_.has_value()) {
        str += std::format("  temperature_hbm_3_max:\n"
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
        str += std::format("  fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(fan_speed_percent, ", "));
    }
    // GPU edge temperature
    if (this->temperature_edge_.has_value()) {
        str += std::format("  temperature_gpu:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_edge_.value(), ", "));
    }
    // GPU hotspot temperature
    if (this->temperature_hotspot_.has_value()) {
        str += std::format("  temperature_hotspot:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_hotspot_.value(), ", "));
    }
    // GPU memory temperature
    if (this->temperature_memory_.has_value()) {
        str += std::format("  temperature_memory:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_memory_.value(), ", "));
    }
    // GPU HBM 0 temperature
    if (this->temperature_hbm_0_.has_value()) {
        str += std::format("  temperature_hbm_0:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_hbm_0_.value(), ", "));
    }
    // GPU HBM 1 temperature
    if (this->temperature_hbm_1_.has_value()) {
        str += std::format("  temperature_hbm_1:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_hbm_1_.value(), ", "));
    }
    // GPU HBM 2 temperature
    if (this->temperature_hbm_2_.has_value()) {
        str += std::format("  temperature_hbm_2:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_hbm_2_.value(), ", "));
    }
    // GPU HBM 3 temperature
    if (this->temperature_hbm_3_.has_value()) {
        str += std::format("  temperature_hbm_3:\n"
                           "    unit: \"m°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_hbm_3_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_temperature_samples &samples) {
    return out << std::format("num_fans [int]: {}\n"
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
                              detail::join(detail::value_or_default(samples.get_fan_speed()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_edge()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hotspot()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_memory()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_0()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_1()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_2()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_3()), ", "));
}

}  // namespace hws
