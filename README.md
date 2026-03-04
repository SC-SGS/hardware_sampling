# hws - Hardware Sampling for CPUs and GPUs

The Hardware Sampling (hws) library can be used to track hardware performance like clock frequency, memory usage,
temperatures, or power draw.
It currently supports CPUs as well as GPUs from NVIDIA, AMD, and Intel.

## Getting Started

### Dependencies

General dependencies:

- a C++17 capable compiler
- [{fmt} > 11.0.2](https://github.com/fmtlib/fmt) for string formatting (automatically build during the CMake
  configuration if it couldn't be found using the respective `find_package` call)
- [Pybind11 > v2.13.1](https://github.com/pybind/pybind11) if Python bindings are enabled (automatically build during
  the CMake configuration if it couldn't be found using the respective `find_package` call)

Dependencies based on the hardware to sample:

- if a CPU should be targeted: at least one of [`turbostat`](https://www.linux.org/docs/man8/turbostat.html) (may
  require root privileges), [`lscpu`](https://man7.org/linux/man-pages/man1/lscpu.1.html), or [
  `free`](https://man7.org/linux/man-pages/man1/free.1.html) and the [
  `subprocess.h`](https://github.com/sheredom/subprocess.h) library (automatically build during the CMake configuration
  if it couldn't be found using the respective `find_package` call)
- if an NVIDIA GPU should be targeted: NVIDIA's Management Library [`NVML`](https://docs.nvidia.com/deploy/nvml-api/)
- if an AMD GPU should be targeted: AMD's ROCm SMI library [
  `rocm_smi_lib`](https://rocm.docs.amd.com/projects/rocm_smi_lib/en/latest/doxygen/html/modules.html)
- if an Intel GPU should be targeted: Intel's [
  `Level Zero library`](https://spec.oneapi.io/level-zero/latest/core/INTRO.html)

### Building hws

To download the hardware sampling use:

```bash
git clone git@github.com:SC-SGS/hardware_sampling.git
cd hardware_sampling 
```

Building the library can be done using the normal CMake approach:

```bash
mkdir build && cd build 
cmake -DCMAKE_BUILD_TYPE=Release [optional_options] .. 
cmake --build . -j
```

#### Optional CMake Options

The `[optional_options]` can be one or multiple of:

- `HWS_ENABLE_CPU_SAMPLING=ON|OFF|AUTO` (default: `AUTO`):
  - `ON`: check whether CPU information can be sampled and fail if this is not the case
  - `AUTO`: check whether CPU information can be sampled but **do not** fail if this is not the case
  - `OFF`: do not check whether CPU information can be sampled

- `HWS_ENABLE_GPU_NVIDIA_SAMPLING=ON|OFF|AUTO` (default: `AUTO`):
  - `ON`: check whether NVIDIA GPU information can be sampled and fail if this is not the case
  - `AUTO`: check whether NVIDIA GPU information can be sampled but **do not** fail if this is not the case
  - `OFF`: do not check whether NVIDIA GPU information can be sampled

- `HWS_ENABLE_GPU_AMD_SAMPLING=ON|OFF|AUTO` (default: `AUTO`):
  - `ON`: check whether AMD GPU information can be sampled and fail if this is not the case
  - `AUTO`: check whether AMD GPU information can be sampled but **do not** fail if this is not the case
  - `OFF`: do not check whether AMD GPU information can be sampled

- `HWS_ENABLE_GPU_INTEL_SAMPLING=ON|OFF|AUTO` (default: `AUTO`):
  - `ON`: check whether Intel GPU information can be sampled and fail if this is not the case
  - `AUTO`: check whether Intel GPU information can be sampled but **do not** fail if this is not the case
  - `OFF`: do not check whether Intel GPU information can be sampled

- `HWS_ENABLE_ERROR_CHECKS=ON|OFF` (default: `OFF`): enable sanity checks during hardware sampling, may be problematic
  with smaller sample intervals
- `HWS_SAMPLING_INTERVAL=100ms` (default: `100ms`): set the sampling interval in milliseconds
- `HWS_ENABLE_PYTHON_BINDINGS=ON|OFF` (default: `ON`): enable Python bindings

### Installing via CMake

The library supports the `install` target:

```bash
cmake --install . --prefix "/home/myuser/installdir"
```

Afterward, the necessary exports should be performed:

```bash
export CMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/share/hws/cmake:${CMAKE_PREFIX_PATH}
export LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64:${LD_LIBRARY_PATH}
export CPLUS_INCLUDE_PATH=${CMAKE_INSTALL_PREFIX}/include:${CPLUS_INCLUDE_PATH}
export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64:${PYTHONPATH}
```

**Note:** when using Intel GPUs, the `CMAKE_MODULE_PATH` should be updated to point to our `cmake` directory containing the
`Findlevel_zero.cmake` file and `export ZES_ENABLE_SYSMAN=1` should be set.

### Installing via pip

The library is also available via pip:

```bash
pip install hardware-sampling
```

This pip install behaves **as if** no additional CMake options were provided.
This means that only the hardware is supported for which the respective vendor libraries was available at the point of the `pip install hardware-sampling` invocation.

## Available samples

The sampling type `fixed` denotes samples that are gathered once per hardware samples like maximum clock frequencies or
temperatures or the total available memory.
The sampling type `sampled` denotes samples that are gathered during the whole hardware sampling process like the
current clock frequencies, temperatures, or memory consumption.

### General samples

| sample              | sample type |    CPUs     | NVIDIA GPUs | AMD GPUs  |  Intel GPUs   |
|:--------------------|:-----------:|:-----------:|:-----------:|:---------:|:-------------:|
| architecture        |    fixed    |     str     |     str     |    str    |       -       |
| byte_order          |    fixed    |     str     |  str (fix)  | str (fix) |   str (fix)   |
| num_cores           |    fixed    |     int     |     int     |     -     |       -       |
| num_threads         |    fixed    |     int     |      -      |     -     |       -       |
| threads_per_core    |    fixed    |     int     |      -      |     -     |       -       |
| cores_per_socket    |    fixed    |     int     |      -      |     -     |       -       |
| num_sockets         |    fixed    |     int     |      -      |     -     |       -       |
| numa_nodes          |    fixed    |     int     |      -      |     -     |       -       |
| vendor_id           |    fixed    |     str     |  str (fix)  |    str    | str (PCIe ID) |
| name                |    fixed    |     str     |     str     |    str    |      str      |
| flags               |    fixed    | list of str |      -      |     -     |  list of str  |
| persistence_mode    |    fixed    |      -      |    bool     |     -     |       -       |
| standby_mode        |    fixed    |      -      |      -      |     -     |      str      |
| num_threads_per_eu  |    fixed    |      -      |      -      |     -     |      int      |
| eu_simd_width       |    fixed    |      -      |      -      |     -     |      int      |
| compute_utilization |   sampled   |      %      |      %      |     %     |       -       |
| memory_utilization  |   sampled   |      -      |      %      |     %     |       -       |
| ipc                 |   sampled   |    float    |      -      |     -     |       -       |
| irq                 |   sampled   |     int     |      -      |     -     |       -       |
| smi                 |   sampled   |     int     |      -      |     -     |       -       |
| poll                |   sampled   |     int     |      -      |     -     |       -       |
| poll_percent        |   sampled   |      %      |      -      |     -     |       -       |
| performance_level   |   sampled   |      -      |     int     |    str    |       -       |

### clock-related samples

| sample                             | sample type | CPUs | NVIDIA GPUs |  AMD GPUs   | Intel GPUs  |
|:-----------------------------------|:-----------:|:----:|:-----------:|:-----------:|:-----------:|
| auto_boosted_clock_enabled         |    fixed    | bool |    bool     |      -      |      -      |
| clock_frequency_min                |    fixed    | MHz  |     MHz     |     MHz     |     MHz     |
| clock_frequency_max                |    fixed    | MHz  |     MHz     |     MHz     |     MHz     |
| memory_clock_frequency_min         |    fixed    |  -   |     MHz     |     MHz     |     MHz     |
| memory_clock_frequency_max         |    fixed    |  -   |     MHz     |     MHz     |     MHz     |
| socket_clock_frequency_min         |    fixed    |  -   |      -      |     MHz     |      -      |
| socket_clock_frequency_min         |    fixed    |  -   |      -      |     MHz     |      -      |
| sm_clock_frequency_max             |    fixed    |  -   |     MHz     |      -      |      -      |
| available_clock_frequencies        |    fixed    |  -   | map of MHz  | list of MHz | list of MHz |
| available_memory_clock_frequencies |    fixed    |  -   | list of MHz | list of MHz | list of MHz |
| clock_frequency                    |   sampled   | MHz  |     MHz     |     MHz     |     MHz     |
| average_non_idle_clock_frequency   |   sampled   | MHz  |      -      |      -      |      -      |
| time_stamp_counter                 |   sampled   | MHz  |      -      |      -      |      -      |
| memory_clock_frequency             |   sampled   |  -   |     MHz     |     MHz     |     MHz     |
| socket_clock_frequency             |   sampled   |  -   |      -      |     MHz     |      -      |
| sm_clock_frequency                 |   sampled   |  -   |     MHz     |      -      |      -      |
| overdrive_level                    |   sampled   |  -   |      -      |      %      |      -      |
| memory_overdrive_level             |   sampled   |  -   |      -      |      %      |      -      |
| throttle_reason                    |   sampled   |  -   |   bitmask   |      -      |   bitmask   |
| throttle_reason_string             |   sampled   |  -   |     str     |      -      |     str     |
| memory_throttle_reason             |   sampled   |  -   |      -      |      -      |   bitmask   |
| memory_throttle_reason_string      |   sampled   |  -   |      -      |      -      |     str     |
| auto_boosted_clock                 |   sampled   |  -   |    bool     |      -      |      -      |
| frequency_limit_tdp                |   sampled   |  -   |      -      |      -      |     MHz     |
| memory_frequency_limit_tdp         |   sampled   |  -   |      -      |      -      |     MHz     |

### power-related samples

| sample                         | sample type |               CPUs                | NVIDIA GPUs |                                        AMD GPUs                                        |                      Intel GPUs                      |
|:-------------------------------|:-----------:|:---------------------------------:|:-----------:|:--------------------------------------------------------------------------------------:|:----------------------------------------------------:|
| power_management_limit         |    fixed    |                 -                 |      W      |                                           W                                            |                          -                           |
| power_enforced_limit           |    fixed    |                 -                 |      W      |                                           W                                            |                          W                           |
| power_measurement_type         |    fixed    |             str (fix)             |     str     |                                          str                                           |                         str                          |
| power_management_mode          |    fixed    |                 -                 |    bool     |                                           -                                            |                         bool                         |
| available_power_profiles       |    fixed    |                 -                 | list of int |                                      list of str                                       |                          -                           |
| power_usage                    |   sampled   |                 W                 |      W      |                                           W                                            | W<br>(calculated via power_total_energy_consumption) |
| system_power_usage             |   sampled   |                 W                 |      W      |                                           -                                            |                          -                           |
| core_watt                      |   sampled   |                 W                 |      -      |                                           -                                            |                          -                           |
| dram_watt                      |   sampled   |                 W                 |      -      |                                           -                                            |                          -                           |
| package_rapl_throttling        |   sampled   |                 %                 |      -      |                                           -                                            |                          -                           |
| dram_rapl_throttling           |   sampled   |                 %                 |      -      |                                           -                                            |                          -                           |
| power_total_energy_consumption |   sampled   | J<br>(calculated via power_usage) |      J      | J<br>(calculated via power_usage if<br>power_total_energy_consumption isn't available) |                          J                           |
| power_profile                  |   sampled   |                 -                 |     int     |                                          str                                           |                          -                           |

### memory-related samples

| sample                      | sample type | CPUs | NVIDIA GPUs | AMD GPUs |           Intel GPUs           |
|:----------------------------|:-----------:|:----:|:-----------:|:--------:|:------------------------------:|
| cache_size_L1d              |    fixed    | str  |      -      |    -     |               -                |
| cache_size_L1i              |    fixed    | str  |      -      |    -     |               -                |
| cache_size_L2               |    fixed    | str  |      -      |    -     |               -                |
| cache_size_L3               |    fixed    | str  |      -      |    -     |               -                |
| memory_total                |    fixed    |  B   |      B      |    B     |  B<br>(map of memory modules)  |
| visible_memory_total        |    fixed    |  -   |      -      |    B     |  B<br>(map of memory modules)  |
| swap_memory_total           |    fixed    |  B   |      -      |    -     |               -                |
| num_pcie_lanes_min          |    fixed    |  -   |      -      |   int    |               -                |
| num_pcie_lanes_max          |    fixed    |  -   |     int     |   int    |              int               |
| pcie_link_generation_max    |    fixed    |  -   |     int     |    -     |              int               |
| pcie_link_speed_max         |    fixed    |  -   |    MBPS     |    -     |              MBPS              |
| pcie_link_transfer_rate_min |    fixed    |  -   |      -      |   MT/s   |               -                |
| pcie_link_transfer_rate_max |    fixed    |  -   |      -      |   MT/s   |               -                |
| memory_bus_width            |    fixed    |  -   |     Bit     |    -     | Bit<br>(map of memory modules) |
| memory_num_channels         |    fixed    |  -   |      -      |    -     | int<br>(map of memory modules) |
| memory_used                 |   sampled   |  B   |      B      |    B     |  B<br>(map of memory modules)  |
| memory_free                 |   sampled   |  B   |      B      |    B     |  B<br>(map of memory modules)  |
| swap_memory_used            |   sampled   |  B   |      -      |    -     |               -                |
| swap_memory_free            |   sampled   |  B   |      -      |    -     |               -                |
| num_pcie_lanes              |   sampled   |  -   |     int     |   int    |              int               |
| pcie_link_generation        |   sampled   |  -   |     int     |    -     |              int               |
| pcie_link_speed             |   sampled   |  -   |    MBPS     |    -     |              MBPS              |
| pcie_link_transfer_rate     |   sampled   |  -   |      -      |   T/s    |               -                |

### temperature-related samples

| sample                  | sample type | CPUs | NVIDIA GPUs | AMD GPUs | Intel GPUs |
|:------------------------|:-----------:|:----:|:-----------:|:--------:|:----------:|
| num_fans                |    fixed    |  -   |     int     |   int    |    int     |
| fan_speed_min           |    fixed    |  -   |      %      |    -     |     -      | 
| fan_speed_max           |    fixed    |  -   |      %      |   RPM    |    RPM     |
| temperature_min         |    fixed    |  -   |      -      |    °C    |     -      |
| temperature_max         |    fixed    |  -   |     °C      |    °C    |     °C     |
| memory_temperature_min  |    fixed    |  -   |      -      |    °C    |     -      |
| memory_temperature_max  |    fixed    |  -   |     °C      |    °C    |     °C     |
| hotspot_temperature_min |    fixed    |  -   |      -      |    °C    |     -      |
| hotspot_temperature_max |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_0_temperature_min   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_0_temperature_max   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_1_temperature_min   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_1_temperature_max   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_2_temperature_min   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_2_temperature_max   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_3_temperature_min   |    fixed    |  -   |      -      |    °C    |     -      |
| hbm_3_temperature_max   |    fixed    |  -   |      -      |    °C    |     -      |
| global_temperature_max  |    fixed    |  -   |      -      |    °C    |     °C     |
| fan_speed_percentage    |   sampled   |  -   |      %      |    %     |     %      |
| temperature             |   sampled   |  °C  |     °C      |    °C    |     °C     |
| memory_temperature      |   sampled   |  -   |      -      |    °C    |     °C     |
| hotspot_temperature     |   sampled   |  -   |      -      |    °C    |     -      |
| hbm_0_temperature       |   sampled   |  -   |      -      |    °C    |     -      |
| hbm_1_temperature       |   sampled   |  -   |      -      |    °C    |     -      |
| hbm_2_temperature       |   sampled   |  -   |      -      |    °C    |     -      |
| hbm_3_temperature       |   sampled   |  -   |      -      |    °C    |     -      |
| global_temperature      |   sampled   |  -   |      -      |    -     |     °C     |
| psu_temperature         |   sampled   |  -   |      -      |    -     |     °C     |
| core_temperature        |   sampled   |  °C  |      -      |    -     |     -      |
| core_throttle_percent   |   sampled   |  %   |      -      |    -     |     -      |

### gfx-related (iGPU) samples

| sample                    | sample type | CPUs |
|:--------------------------|:-----------:|:----:|
| gfx_render_state_percent  |   sampled   |  %   |
| gfx_frequency             |   sampled   | MHz  |
| average_gfx_frequency     |   sampled   | MHz  |
| gfx_state_c0_percent      |   sampled   |  %   |
| cpu_works_for_gpu_percent |   sampled   |  %   |
| gfx_watt                  |   sampled   |  W   |

### "idle states"-related samples

| sample                               | sample type |     CPUs      |
|:-------------------------------------|:-----------:|:-------------:|
| idle_states                          |    fixed    | map of values |
| all_cpus_state_c0_percent            |   sampled   |       %       |
| any_cpu_state_c0_percent             |   sampled   |       %       |
| low_power_idle_state_percent         |   sampled   |       %       |
| system_low_power_idle_state_percent  |   sampled   |       %       |
| package_low_power_idle_state_percent |   sampled   |       %       |

## Example Python usage

```python
import HardwareSampling as hws
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import datetime

sampler = hws.CpuHardwareSampler()
# could also be, e.g.,
# sampler = hws.GpuNvidiaHardwareSampler()
sampler.start()

sampler.add_event("init")
A = np.random.rand(2 ** 14, 2 ** 14)
B = np.random.rand(2 ** 14, 2 ** 14)

sampler.add_event("matmul")
C = A @ B

sampler.stop()
sampler.dump_yaml("track.yaml")

# plot the results
time_points = sampler.relative_time_points()

plt.plot(time_points, sampler.clock_samples().get_clock_frequency(), label="average")
plt.plot(time_points, sampler.clock_samples().get_average_non_idle_clock_frequency(), label="average non-idle")

axes = plt.gcf().axes[0]
x_bounds = axes.get_xlim()
for event in sampler.get_relative_events()[1:-1]:
    axes.axvline(x=event.relative_time_point, color='r')
    axes.annotate(text=event.name,
                  xy=(((event.relative_time_point - x_bounds[0]) / (x_bounds[1] - x_bounds[0])), 1.025),
                  xycoords='axes fraction', rotation=270)

plt.xlabel("runtime [ms]")
plt.ylabel("clock frequency [MHz]")
plt.legend()
plt.show()
```

<p align="center">
  <img alt="example frequency plot" src="https://github.com/SC-SGS/hardware_sampling/raw/main/.figures/clock_frequency.png" width="75%">
</p>

## License

The hws library is distributed under
the [MIT license](https://github.com/SC-SGS/hardware_sampling/blob/main/LICENSE.md).
