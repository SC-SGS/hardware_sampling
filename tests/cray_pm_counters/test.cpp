/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Simple, dependency-free regression checks for the cray_pm_counters backend.
 * @details Uses the HWS_PM_COUNTERS_PATH environment variable override to point the backend at synthetic
 *          directories instead of the real (Cray-only) /sys/cray/pm_counters, so these checks run on any Linux
 *          machine. The synthetic content mirrors a real dump gathered on an HLRS "Hunter" node (HPE Cray EX255a,
 *          PM counters version 3, see scripts/hwmon_probe.pbs). Not a general-purpose test framework - just enough to
 *          catch a regression in the specific bugs found (and fixed) during code review.
 */

#include "hws/core.hpp"

#include <chrono>    // std::chrono::milliseconds
#include <cstdlib>   // std::exit, setenv
#include <filesystem>  // std::filesystem
#include <fstream>   // std::ofstream
#include <iostream>  // std::cout, std::cerr
#include <string>    // std::string
#include <thread>    // std::this_thread::sleep_for

namespace {

int g_failures = 0;

void check(bool condition, const std::string &description) {
    if (condition) {
        std::cout << "  ok:     " << description << "\n";
    } else {
        std::cerr << "  FAILED: " << description << "\n";
        ++g_failures;
    }
}

void write_file(const std::filesystem::path &path, const std::string &content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file{ path };
    file << content;
}

void set_pm_counters_path(const std::filesystem::path &path) {
    setenv("HWS_PM_COUNTERS_PATH", path.c_str(), 1);
}

// mirrors the real dump gathered on Hunter (hwmon_probe.o250927): 14 of the 23 real files, enough to exercise
// every classification/parsing rule without duplicating the full dump
std::filesystem::path make_synthetic_pm_counters(const std::filesystem::path &root) {
    std::filesystem::remove_all(root);
    write_file(root / "energy", "12044825101 J 7810213855886 us\n");
    write_file(root / "power", "724 W 7810213855886 us\n");
    write_file(root / "power_cap", "2894 W\n");
    write_file(root / "accel0_energy", "2869283299 J 7810213855886 us\n");
    write_file(root / "accel0_power", "212 W 7810213855886 us\n");
    write_file(root / "accel0_power_cap", "0 W\n");
    write_file(root / "capped_energy", "12752953 J 7810213855886 us\n");
    write_file(root / "overshoot", "62408\n");
    write_file(root / "overshoot_energy", "3951201 J 7810213855886 us\n");
    write_file(root / "freshness", "60043284\n");
    write_file(root / "generation", "16\n");
    write_file(root / "raw_scan_hz", "10\n");
    write_file(root / "startup", "1778580757634187\n");
    write_file(root / "version", "3\n");
    return root;
}

void test_absent() {
    std::cout << "test_absent\n";
    set_pm_counters_path("/nonexistent/pm_counters_for_hws_test");

    hws::cray_pm_counters_hardware_sampler sampler{};
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    sampler.stop_sampling();

    check(!sampler.general_samples().has_samples(), "no general samples when pm_counters is absent");
    check(!sampler.power_samples().has_samples(), "no power samples when pm_counters is absent");
}

void test_real_values(const std::filesystem::path &root) {
    std::cout << "test_real_values\n";
    set_pm_counters_path(root);

    hws::cray_pm_counters_hardware_sampler sampler{};
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    sampler.stop_sampling();

    const auto &energy = sampler.power_samples().get_energy_counters();
    const auto &power = sampler.power_samples().get_power_counters();
    const auto &energy_ts = sampler.power_samples().get_energy_timestamps_us();
    const auto &metadata = sampler.general_samples().get_metadata();

    check(energy.has_value() && energy->count("energy") == 1 && energy->at("energy").front() == 12044825101ull,
          "'energy' discovered and parsed as a measured energy counter");
    check(energy.has_value() && energy->count("accel0_energy") == 1,
          "'accel0_energy' discovered as a measured energy counter");
    check(power.has_value() && power->count("power") == 1 && power->at("power").front() == 724ull,
          "'power' discovered and parsed as a measured power counter");
    check(energy_ts.has_value() && energy_ts->count("energy") == 1 && energy_ts->at("energy").front() == 7810213855886ull,
          "'energy's embedded HSS timestamp extracted correctly");

    // caps and derived/event counters must not pollute the measured telemetry maps (regression: Opus review finding)
    check(!energy.has_value() || energy->count("capped_energy") == 0, "'capped_energy' excluded from energy_counters");
    check(!energy.has_value() || energy->count("overshoot_energy") == 0, "'overshoot_energy' excluded from energy_counters");
    check(!power.has_value() || power->count("power_cap") == 0, "'power_cap' excluded from power_counters");
    check(!power.has_value() || power->count("accel0_power_cap") == 0, "'accel0_power_cap' excluded from power_counters");

    // ... they should show up as general metadata instead
    check(metadata.has_value() && metadata->count("power_cap") == 1, "'power_cap' present in general metadata");
    check(metadata.has_value() && metadata->count("capped_energy") == 1, "'capped_energy' present in general metadata");
    check(metadata.has_value() && metadata->count("overshoot") == 1, "'overshoot' present in general metadata");
    // freshness must be re-sampled every tick, not read once (regression check)
    check(metadata.has_value() && metadata->count("freshness") == 1 && metadata->at("freshness").size() > 1,
          "'freshness' is re-sampled every tick, not just once");
}

void test_adversarial_names(const std::filesystem::path &root) {
    std::cout << "test_adversarial_names\n";
    write_file(root / "power_state", "ok\n");
    write_file(root / "energy_source", "grid\n");
    set_pm_counters_path(root);

    hws::cray_pm_counters_hardware_sampler sampler{};
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    sampler.stop_sampling();

    const auto &energy = sampler.power_samples().get_energy_counters();
    const auto &power = sampler.power_samples().get_power_counters();
    const auto &metadata = sampler.general_samples().get_metadata();

    // regression check (GPT-5.5 review finding): "energy"/"power" must be a path-segment suffix, not any substring
    check(!energy.has_value() || energy->count("energy_source") == 0, "'energy_source' not misclassified as an energy counter");
    check(!power.has_value() || power->count("power_state") == 0, "'power_state' not misclassified as a power counter");
    check(metadata.has_value() && metadata->count("power_state") == 1, "'power_state' present in general metadata");
    check(metadata.has_value() && metadata->count("energy_source") == 1, "'energy_source' present in general metadata");
}

void test_nested_path(const std::filesystem::path &root) {
    std::cout << "test_nested_path\n";
    write_file(root / "nested" / "energy", "111 J 999 us\n");
    set_pm_counters_path(root);

    hws::cray_pm_counters_hardware_sampler sampler{};
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    sampler.stop_sampling();

    const auto &energy = sampler.power_samples().get_energy_counters();
    // regression check (Opus review finding): nested files use their real relative path as key, not a flattened one
    check(energy.has_value() && energy->count("nested/energy") == 1, "nested file uses its un-flattened relative path as map key");
}

void test_throttle(const std::filesystem::path &root) {
    std::cout << "test_throttle\n";
    set_pm_counters_path(root);

    // request a 10ms interval, far faster than the synthetic hardware's 10 Hz (raw_scan_hz=10); the sampler must
    // throttle to the hardware rate instead of hammering the (synthetic) sysfs files
    hws::cray_pm_counters_hardware_sampler sampler{ std::chrono::milliseconds(10) };
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(650));
    sampler.stop_sampling();

    bool throttled = false;
    for (const auto &e : sampler.get_events()) {
        if (e.name.find("throttled") != std::string::npos) {
            throttled = true;
        }
    }
    check(throttled, "a throttling event is recorded for a too-fast configured interval");

    const auto &energy = sampler.power_samples().get_energy_counters();
    const std::size_t num_samples = (energy.has_value() && energy->count("energy") == 1) ? energy->at("energy").size() : 0;
    // ~650ms / 100ms (hardware period) ~= 6-8 samples; a buggy 10ms cadence would produce ~65
    check(num_samples > 0 && num_samples < 20, "sample count matches the hardware-throttled cadence, not the requested 10ms one");
}

void test_throttle_power_only_category(const std::filesystem::path &root) {
    std::cout << "test_throttle_power_only_category\n";
    set_pm_counters_path(root);

    // regression check (GPT-5.5 review finding): throttling must not depend on sample_category::general being enabled
    hws::cray_pm_counters_hardware_sampler sampler{ std::chrono::milliseconds(10), hws::sample_category::power };
    sampler.start_sampling();
    std::this_thread::sleep_for(std::chrono::milliseconds(650));
    sampler.stop_sampling();

    const auto &energy = sampler.power_samples().get_energy_counters();
    const std::size_t num_samples = (energy.has_value() && energy->count("energy") == 1) ? energy->at("energy").size() : 0;
    check(num_samples > 0 && num_samples < 20, "throttling still applies when only sample_category::power is enabled");
}

}  // namespace

int main() {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "hws_cray_pm_counters_test";

    test_absent();
    make_synthetic_pm_counters(root);
    test_real_values(root);
    test_adversarial_names(root);
    test_nested_path(root);
    test_throttle(root);
    test_throttle_power_only_category(root);

    std::filesystem::remove_all(root);

    if (g_failures > 0) {
        std::cerr << "\n" << g_failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "\nall checks passed\n";
    return 0;
}
