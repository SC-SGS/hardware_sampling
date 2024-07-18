# hws - Hardware Sampling for CPUs and GPUs

The Hardware Sampling (hws) library can be used to track hardware performance like clock frequency, memory usage, temperatures, or power draw. 
It currently supports CPUs as well as GPUs from NVIDIA, AMD, and Intel.

## Getting Started

### Dependencies

General dependencies:

- a C++20 capable compiler supporting `std::format` (tested with GCC 14.1.0)
- [Pybind11 > v2.13.1](https://github.com/pybind/pybind11) if Python bindings are enabled (automatically build during the CMake configuration if it couldn't be found using the respective `find_package` call)

Dependencies based on the hardware to sample:

- if a CPU should be targeted: at least one of [`turbostat`](https://www.linux.org/docs/man8/turbostat.html) (may require root privileges), [`lscpu`](https://man7.org/linux/man-pages/man1/lscpu.1.html), or [`free`](https://man7.org/linux/man-pages/man1/free.1.html) and the [`subprocess.h`](https://github.com/sheredom/subprocess.h) library (automatically build during the CMake configuration if it couldn't be found using the respective `find_package` call)
- if an NVIDIA GPU should be targeted: NVIDIA's Management Library [`NVML`](https://docs.nvidia.com/deploy/nvml-api/)
- if an AMD GPU should be targeted: AMD's ROCm SMI library [`rocm_smi_lib`](https://rocm.docs.amd.com/projects/rocm_smi_lib/en/latest/doxygen/html/modules.html)
- if an Intel GPU should be targeted: Intel's [`Level Zero library`](https://spec.oneapi.io/level-zero/latest/core/INTRO.html)

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

- `HWS_ENABLE_ERROR_CHECKS=ON|OFF` (default: `OFF`): enable sanity checks during hardware sampling, may be problematic with smaller sample intervals
- `HWS_SAMPLING_INTERVAL=100ms` (default: `100ms`): set the sampling interval in milliseconds
- `HWS_ENABLE_PYTHON_BINDINGS=ON|OFF` (default: `ON`): enable Python bindings

### Installing

The library supports the `install` target:

```bash
cmake --install . --prefix "/home/myuser/installdir"
```

Afterward, the necessary exports should be performed:

```bash
export CMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/share/hardware_sampling/cmake:${CMAKE_PREFIX_PATH}
export LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:${LD_LIBRARY_PATH}
export CPLUS_INCLUDE_PATH=${CMAKE_INSTALL_PREFIX}/include:${CPLUS_INCLUDE_PATH}
export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib:${PYTHONPATH}
```

## Example Python usage

```python
import HardwareSampling
import numpy as np
import matplotlib.pyplot as plt
import datetime

sampler = HardwareSampling.CpuHardwareSampler()
# could also be, e.g.,
# sampler = HardwareSampling.GpuNvidiaHardwareSampler()
sampler.start()

sampler.add_event("init")
A = np.random.rand(2**14, 2**14)
B = np.random.rand(2**14, 2**14)

sampler.add_event("matmul")
C = A @ B

sampler.stop()
sampler.dump_yaml("track.yaml")

# plot the results
time_points = sampler.time_points()
relative_time_points = [(t - time_points[0]) / datetime.timedelta(milliseconds=1) for t in time_points]

plt.plot(relative_time_points, sampler.clock_samples().get_average_frequency(), label="average")
plt.plot(relative_time_points, sampler.clock_samples().get_average_non_idle_frequency(), label="average non-idle")

axes = plt.gcf().axes[0]
x_bounds = axes.get_xlim()
for event in sampler.get_events()[1:-1]:
    tp = (event.time_point - time_points[0]) / datetime.timedelta(milliseconds=1)

    axes.axvline(x=tp, color='r')
    axes.annotate(text=event.name, xy=(((tp - x_bounds[0]) / (x_bounds[1] - x_bounds[0])), 1.025), xycoords='axes fraction', rotation=270)

plt.xlabel("runtime [ms]")
plt.ylabel("clock frequency [MHz]")
plt.legend()
plt.show()
```

<p align="center">
  <img alt="example frequency plot" src=".figures/clock_frequency.png" width="75%">
</p>

## License

The hws library is distributed under the [MIT license](https://github.com/SC-SGS/hardware_sampling/blob/main/LICENSE.md).