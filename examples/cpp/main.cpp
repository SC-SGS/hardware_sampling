/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/core.hpp"

#include <cstddef>  // std::size_t
#include <numeric>  // std::iota
#include <vector>   // std::vector

int main() {
    hws::cpu_hardware_sampler sampler{};
    // could also be, e.g.,
    // hws::gpu_nvidia_hardware_sampler sampler{};
    sampler.start_sampling();

    sampler.add_event("init");
    const std::size_t m_size = 2048;
    const std::size_t n_size = 2048;
    const std::size_t k_size = 2048;

    std::vector<double> A(m_size * k_size);
    std::vector<double> B(k_size * n_size);
    std::vector<double> C(m_size * n_size);

    std::iota(A.begin(), A.end(), 0);
    std::iota(B.begin(), B.end(), 1);

    sampler.add_event("matmul");
    for (std::size_t m = 0; m < m_size; ++m) {
        for (std::size_t n = 0; n < n_size; ++n) {
            for (std::size_t k = 0; k < k_size; ++k) {
                C[m * n_size + n] += A[m * k_size + k] * B[k * n_size + n];
            }
        }
    }

    sampler.stop_sampling();
    sampler.dump_yaml("track.yaml");

    return 0;
}
