/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_amd/rocm_smi_samples.hpp"

#include "hws/utility.hpp"  // hws::detail::{value_or_default, quote}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

bool rocm_smi_general_samples::has_samples() const {
    return this->architecture_.has_value() || this->byte_order_.has_value() || this->vendor_id_.has_value() || this->name_.has_value()
           || this->compute_utilization_.has_value() || this->memory_utilization_.has_value() || this->performance_level_.has_value();
}

std::string rocm_smi_general_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(detail::quote(this->performance_level_.value()), ", "));
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_general_samples &samples) {
    return out << fmt::format("architecture [string]: {}\n"
                              "byte_order [string]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "compute_utilization [%]: [{}]\n"
                              "memory_utilization [%]: [{}]\n"
                              "performance_level [string]: [{}]",
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

bool rocm_smi_clock_samples::has_samples() const {
    return this->clock_frequency_min_.has_value() || this->clock_frequency_max_.has_value() || this->memory_clock_frequency_min_.has_value()
           || this->memory_clock_frequency_max_.has_value() || this->socket_clock_frequency_min_.has_value() || this->socket_clock_frequency_max_.has_value()
           || this->available_clock_frequencies_.has_value() || this->available_memory_clock_frequencies_.has_value() || this->clock_frequency_.has_value()
           || this->memory_clock_frequency_.has_value() || this->socket_clock_frequency_.has_value() || this->overdrive_level_.has_value()
           || this->memory_overdrive_level_.has_value();
}

std::string rocm_smi_clock_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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

bool rocm_smi_power_samples::has_samples() const {
    return this->power_management_limit_.has_value() || this->power_enforced_limit_.has_value() || this->power_measurement_type_.has_value()
           || this->available_power_profiles_.has_value() || this->power_usage_.has_value() || this->power_total_energy_consumption_.has_value()
           || this->power_profile_.has_value();
}

std::string rocm_smi_power_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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
                           fmt::join(detail::quote(this->available_power_profiles_.value()), ", "));
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
                           fmt::join(detail::quote(this->power_profile_.value()), ", "));
    }

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

bool rocm_smi_memory_samples::has_samples() const {
    return this->memory_total_.has_value() || this->visible_memory_total_.has_value() || this->num_pcie_lanes_min_.has_value()
           || this->num_pcie_lanes_max_.has_value() || this->pcie_link_transfer_rate_min_.has_value() || this->pcie_link_transfer_rate_max_.has_value()
           || this->memory_used_.has_value() || this->memory_free_.has_value() || this->num_pcie_lanes_.has_value() || this->pcie_link_transfer_rate_.has_value();
}

std::string rocm_smi_memory_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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
    if (this->num_pcie_lanes_min_.has_value()) {
        str += fmt::format("  num_pcie_lanes_min:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_pcie_lanes_min_.value());
    }
    // max number of PCIe lanes
    if (this->num_pcie_lanes_max_.has_value()) {
        str += fmt::format("  num_pcie_lanes_max:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_pcie_lanes_max_.value());
    }
    // the minimum PCIe link transfer rate
    if (this->pcie_link_transfer_rate_min_.has_value()) {
        str += fmt::format("  pcie_link_transfer_rate_min:\n"
                           "    unit: \"MT/s\"\n"
                           "    values: {}\n",
                           this->pcie_link_transfer_rate_min_.value());
    }
    // the maximum PCIe link transfer rate
    if (this->pcie_link_transfer_rate_max_.has_value()) {
        str += fmt::format("  pcie_link_transfer_rate_max:\n"
                           "    unit: \"MT/s\"\n"
                           "    values: {}\n",
                           this->pcie_link_transfer_rate_max_.value());
    }

    // used memory
    if (this->memory_used_.has_value()) {
        str += fmt::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_used_.value(), ", "));
    }
    // free memory
    if (this->memory_free_.has_value()) {
        str += fmt::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_free_.value(), ", "));
    }

    // number of PCIe lanes
    if (this->num_pcie_lanes_.has_value()) {
        str += fmt::format("  num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->num_pcie_lanes_.value(), ", "));
    }
    // PCIe transfer rate
    if (this->pcie_link_transfer_rate_.has_value()) {
        str += fmt::format("  pcie_link_transfer_rate:\n"
                           "    unit: \"MT/s\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_transfer_rate_.value(), ", "));
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_memory_samples &samples) {
    return out << fmt::format("memory_total [B]: {}\n"
                              "visible_memory_total [B]: {}\n"
                              "num_pcie_lanes_min [int]: {}\n"
                              "num_pcie_lanes_max [int]: {}\n"
                              "pcie_link_transfer_rate_min [MBPS]: {}\n"
                              "pcie_link_transfer_rate_max [MBPS]: {}\n"
                              "memory_used [B]: [{}]\n"
                              "memory_free [B]: [{}]\n"
                              "num_pcie_lanes [int]: [{}]\n"
                              "pcie_link_transfer_rate [MBPS]: [{}]",
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_visible_memory_total()),
                              detail::value_or_default(samples.get_num_pcie_lanes_min()),
                              detail::value_or_default(samples.get_num_pcie_lanes_max()),
                              detail::value_or_default(samples.get_pcie_link_transfer_rate_min()),
                              detail::value_or_default(samples.get_pcie_link_transfer_rate_max()),
                              fmt::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              fmt::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_transfer_rate()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

bool rocm_smi_temperature_samples::has_samples() const {
    return this->num_fans_.has_value() || this->fan_speed_max_.has_value() || this->temperature_min_.has_value() || this->temperature_max_.has_value()
           || this->memory_temperature_min_.has_value() || this->memory_temperature_max_.has_value() || this->hotspot_temperature_min_.has_value()
           || this->hotspot_temperature_max_.has_value() || this->hbm_0_temperature_min_.has_value() || this->hbm_0_temperature_max_.has_value()
           || this->hbm_1_temperature_min_.has_value() || this->hbm_1_temperature_max_.has_value() || this->hbm_2_temperature_min_.has_value()
           || this->hbm_2_temperature_max_.has_value() || this->hbm_3_temperature_min_.has_value() || this->hbm_3_temperature_max_.has_value()
           || this->fan_speed_percentage_.has_value() || this->temperature_.has_value() || this->memory_temperature_.has_value()
           || this->hotspot_temperature_.has_value() || this->hbm_0_temperature_.has_value() || this->hbm_1_temperature_.has_value()
           || this->hbm_2_temperature_.has_value() || this->hbm_3_temperature_.has_value();
}

std::string rocm_smi_temperature_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "temperature:\n" };

    // number of fans (emulated)
    if (this->num_fans_.has_value()) {
        str += fmt::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // maximum fan speed
    if (this->fan_speed_max_.has_value()) {
        str += fmt::format("  fan_speed_max:\n"
                           "    unit: \"RPM\"\n"
                           "    values: {}\n",
                           this->fan_speed_max_.value());
    }
    // minimum GPU edge temperature
    if (this->temperature_min_.has_value()) {
        str += fmt::format("  temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_min_.value());
    }
    // maximum GPU edge temperature
    if (this->temperature_max_.has_value()) {
        str += fmt::format("  temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_max_.value());
    }
    // minimum GPU memory temperature
    if (this->memory_temperature_min_.has_value()) {
        str += fmt::format("  memory_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->memory_temperature_min_.value());
    }
    // maximum GPU memory temperature
    if (this->memory_temperature_max_.has_value()) {
        str += fmt::format("  memory_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->memory_temperature_max_.value());
    }
    // minimum GPU hotspot temperature
    if (this->hotspot_temperature_min_.has_value()) {
        str += fmt::format("  hotspot_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hotspot_temperature_min_.value());
    }
    // maximum GPU hotspot temperature
    if (this->hotspot_temperature_max_.has_value()) {
        str += fmt::format("  hotspot_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hotspot_temperature_max_.value());
    }
    // minimum GPU HBM 0 temperature
    if (this->hbm_0_temperature_min_.has_value()) {
        str += fmt::format("  hbm_0_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_0_temperature_min_.value());
    }
    // maximum GPU HBM 0 temperature
    if (this->hbm_0_temperature_max_.has_value()) {
        str += fmt::format("  hbm_0_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_0_temperature_max_.value());
    }
    // minimum GPU HBM 1 temperature
    if (this->hbm_1_temperature_min_.has_value()) {
        str += fmt::format("  hbm_1_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_1_temperature_min_.value());
    }
    // maximum GPU HBM 1 temperature
    if (this->hbm_1_temperature_max_.has_value()) {
        str += fmt::format("  hbm_1_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_1_temperature_max_.value());
    }
    // minimum GPU HBM 2 temperature
    if (this->hbm_2_temperature_min_.has_value()) {
        str += fmt::format("  hbm_2_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_2_temperature_min_.value());
    }
    // maximum GPU HBM 2 temperature
    if (this->hbm_2_temperature_max_.has_value()) {
        str += fmt::format("  hbm_2_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_2_temperature_max_.value());
    }
    // minimum GPU HBM 3 temperature
    if (this->hbm_3_temperature_min_.has_value()) {
        str += fmt::format("  hbm_3_temperature_min:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_3_temperature_min_.value());
    }
    // maximum GPU HBM 3 temperature
    if (this->hbm_3_temperature_max_.has_value()) {
        str += fmt::format("  hbm_3_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->hbm_3_temperature_max_.value());
    }

    // fan speed
    if (this->fan_speed_percentage_.has_value()) {
        str += fmt::format("  fan_speed_percentage:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->fan_speed_percentage_.value(), ", "));
    }
    // GPU edge temperature
    if (this->temperature_.has_value()) {
        str += fmt::format("  temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_.value(), ", "));
    }
    // GPU memory temperature
    if (this->memory_temperature_.has_value()) {
        str += fmt::format("  memory_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_temperature_.value(), ", "));
    }
    // GPU hotspot temperature
    if (this->hotspot_temperature_.has_value()) {
        str += fmt::format("  hotspot_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->hotspot_temperature_.value(), ", "));
    }
    // GPU HBM 0 temperature
    if (this->hbm_0_temperature_.has_value()) {
        str += fmt::format("  hbm_0_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->hbm_0_temperature_.value(), ", "));
    }
    // GPU HBM 1 temperature
    if (this->hbm_1_temperature_.has_value()) {
        str += fmt::format("  hbm_1_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->hbm_1_temperature_.value(), ", "));
    }
    // GPU HBM 2 temperature
    if (this->hbm_2_temperature_.has_value()) {
        str += fmt::format("  hbm_2_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->hbm_2_temperature_.value(), ", "));
    }
    // GPU HBM 3 temperature
    if (this->hbm_3_temperature_.has_value()) {
        str += fmt::format("  hbm_3_temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->hbm_3_temperature_.value(), ", "));
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const rocm_smi_temperature_samples &samples) {
    return out << fmt::format("num_fans [int]: {}\n"
                              "fan_speed_max [RPM]: {}\n"
                              "temperature_min [°C]: {}\n"
                              "temperature_max [°C]: {}\n"
                              "memory_temperature_min [°C]: {}\n"
                              "memory_temperature_max [°C]: {}\n"
                              "hotspot_temperature_min [°C]: {}\n"
                              "hotspot_temperature_max [°C]: {}\n"
                              "hbm_0_temperature_min [°C]: {}\n"
                              "hbm_0_temperature_max [°C]: {}\n"
                              "hbm_1_temperature_min [°C]: {}\n"
                              "hbm_1_temperature_max [°C]: {}\n"
                              "hbm_2_temperature_min [°C]: {}\n"
                              "hbm_2_temperature_max [°C]: {}\n"
                              "hbm_3_temperature_min [°C]: {}\n"
                              "hbm_3_temperature_max [°C]: {}\n"
                              "fan_speed_percentage [%]: [{}]\n"
                              "temperature [°C]: [{}]\n"
                              "memory_temperature [°C]: [{}]\n"
                              "hotspot_temperature [°C]: [{}]\n"
                              "hbm_0_temperature [°C]: [{}]\n"
                              "hbm_1_temperature [°C]: [{}]\n"
                              "hbm_2_temperature [°C]: [{}]\n"
                              "hbm_3_temperature [°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_fan_speed_max()),
                              detail::value_or_default(samples.get_temperature_min()),
                              detail::value_or_default(samples.get_temperature_max()),
                              detail::value_or_default(samples.get_memory_temperature_min()),
                              detail::value_or_default(samples.get_memory_temperature_max()),
                              detail::value_or_default(samples.get_hotspot_temperature_min()),
                              detail::value_or_default(samples.get_hotspot_temperature_max()),
                              detail::value_or_default(samples.get_hbm_0_temperature_min()),
                              detail::value_or_default(samples.get_hbm_0_temperature_max()),
                              detail::value_or_default(samples.get_hbm_1_temperature_min()),
                              detail::value_or_default(samples.get_hbm_1_temperature_max()),
                              detail::value_or_default(samples.get_hbm_2_temperature_min()),
                              detail::value_or_default(samples.get_hbm_2_temperature_max()),
                              detail::value_or_default(samples.get_hbm_3_temperature_min()),
                              detail::value_or_default(samples.get_hbm_3_temperature_max()),
                              fmt::join(detail::value_or_default(samples.get_fan_speed_percentage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_hotspot_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_hbm_0_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_hbm_1_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_hbm_2_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_hbm_3_temperature()), ", "));
}

}  // namespace hws
