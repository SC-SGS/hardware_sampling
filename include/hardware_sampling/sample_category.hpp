/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines an enum class with all sample categories to be able to only selectively enable some samples.
 */

#ifndef HARDWARE_SAMPLING_SAMPLE_CATEGORY_HPP_
#define HARDWARE_SAMPLING_SAMPLE_CATEGORY_HPP_
#pragma once

namespace hws {

/**
 * @brief Enum class as bitfield containing the possible sample categories.
 * @details The sample_category "gfx" and "idle_state" are only used in the cpu_hardware_sampler.
 *          Additionally, the "all" sample_category is available to easily enable all hardware samples (default).
 */
enum class sample_category : int {
    // clang-format off
    /// General hardware samples like architecture, names, or utilization.
    general     = 0b00000001,
    /// Clock-related hardware samples like minimum, maximum, and current frequencies or throttle reasons.
    clock       = 0b00000010,
    /// Power-related hardware samples like current power draw or total energy consumption.
    power       = 0b00000100,
    /// Memory-related hardware samples like memory usage or PCIe information.
    memory      = 0b00001000,
    /// Temperature-related hardware samples like maximum and current temperatures.
    temperature = 0b00010000,
    /// Gfx-related (iGPU) hardware samples. Only used in the cpu_hardware_sampler.
    gfx         = 0b00100000,
    /// Idle-state-related hardware samples. Only used in the cpu_hardware_sampler.
    idle_state  = 0b01000000,
    /// Shortcut to enable all available hardware samples (default).
    all         = 0b01111111
    // clang-format on
};

/**
 * @brief Compute the bitwise not of @p sc.
 * @param[in] sc the sample_category to apply the bitwise not to
 * @return the bitwise not result (`[[nodiscard]]`)
 */
[[nodiscard]] constexpr sample_category operator~(const sample_category sc) noexcept {
    return static_cast<sample_category>(~static_cast<int>(sc));
}

/**
 * @brief Compute the bitwise and between @p lhs and @p rhs and return a new sample_category.
 * @param[in] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return the bitwise and result (`[[nodiscard]]`)
 */
[[nodiscard]] constexpr sample_category operator&(const sample_category lhs, const sample_category rhs) noexcept {
    return static_cast<sample_category>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

/**
 * @brief Compute the bitwise or between @p lhs and @p rhs and return a new sample_category.
 * @param[in] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return the bitwise or result (`[[nodiscard]]`)
 */
[[nodiscard]] constexpr sample_category operator|(const sample_category lhs, const sample_category rhs) noexcept {
    return static_cast<sample_category>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

/**
 * @brief Compute the bitwise xor between @p lhs and @p rhs and return a new sample_category.
 * @param[in] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return the bitwise xor result (`[[nodiscard]]`)
 */
[[nodiscard]] constexpr sample_category operator^(const sample_category lhs, const sample_category rhs) noexcept {
    return static_cast<sample_category>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

/**
 * @brief Compute the bitwise compound and between @p lhs and @p rhs and return the result in @p lhs.
 * @param[in,out] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return a reference to @p lhs containing the bitwise and result
 */
constexpr sample_category &operator&=(sample_category &lhs, const sample_category rhs) noexcept {
    lhs = lhs & rhs;
    return lhs;
}

/**
 * @brief Compute the bitwise compound or between @p lhs and @p rhs and return the result in @p lhs.
 * @param[in,out] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return a reference to @p lhs containing the bitwise or result
 */
constexpr sample_category &operator|=(sample_category &lhs, const sample_category rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}

/**
 * @brief Compute the bitwise compound xor between @p lhs and @p rhs and return the result in @p lhs.
 * @param[in,out] lhs the first sample_category
 * @param[in] rhs the second sample_category
 * @return a reference to @p lhs containing the bitwise xor result
 */
constexpr sample_category &operator^=(sample_category &lhs, const sample_category rhs) noexcept {
    lhs = lhs ^ rhs;
    return lhs;
}

}  // namespace hws

#endif  // HARDWARE_SAMPLING_SAMPLE_CATEGORY_HPP_
