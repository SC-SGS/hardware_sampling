/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/utility.hpp"

#include <algorithm>    // std::min, std::transform, std::all_of
#include <cctype>       // std::tolower, std::isdigit
#include <sstream>      // std::stringstream
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include <mpi.h>  // MPI_Comm, MPI_Gatherv, MPI_Gather, MPI_Initialized, MPI_Comm_rank, MPI_Comm_size
#endif

namespace hws::detail {

bool starts_with(const std::string_view sv, const std::string_view start) noexcept {
    return sv.substr(0, start.size()) == start;
}

std::string_view trim(std::string_view str) noexcept {
    // trim right
    {
        const std::string_view::size_type pos = std::min(str.find_last_not_of(" \t\v\r\n\f") + 1, str.size());
        str = str.substr(0, pos);
    }
    // trim left
    {
        const std::string_view::size_type pos = std::min(str.find_first_not_of(" \t\v\r\n\f"), str.size());
        str = str.substr(pos);
    }
    return str;
}

std::string to_lower_case(const std::string_view str) {
    std::string lowercase_str{ str };
    std::transform(str.begin(), str.end(), lowercase_str.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(static_cast<int>(c))); });
    return lowercase_str;
}

std::vector<std::string_view> split(const std::string_view str, const char delim) {
    std::vector<std::string_view> split_str;

    // if the input str is empty, return an empty vector
    if (str.empty()) {
        return split_str;
    }

    std::string_view::size_type pos = 0;
    std::string_view::size_type next = 0;
    while (next != std::string_view::npos) {
        next = str.find_first_of(delim, pos);
        split_str.emplace_back(next == std::string_view::npos ? str.substr(pos) : str.substr(pos, next - pos));
        pos = next + 1;
    }
    return split_str;
}

bool is_integer(std::string_view str) {
    return std::all_of(str.cbegin(), str.cend(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
}

std::string indent_lines(const std::string &text, const std::string &prefix) {
    std::stringstream ss{ text };

    std::string line;
    std::string out;

    while (std::getline(ss, line)) {
        out += prefix + line + '\n';
    }

    return out;
}

#if defined(HWS_MPI_SUPPORT_ENABLED)
std::string gather_yaml_strings_mpi(const std::string &local_yaml, MPI_Comm communicator) {
    int initialized = 0;
    MPI_Initialized(&initialized);

    if (!initialized) {
        throw std::runtime_error("MPI must already be initialized");
    }

    // MPI rank and world size for identification and communication
    int rank = 0, world_size = 0;
    MPI_Comm_rank(communicator, &rank);
    MPI_Comm_size(communicator, &world_size);

    // gather the size of the yaml string from each rank
    int local_size = static_cast<int>(local_yaml.size());

    std::vector<int> recv_sizes;

    if (rank == 0) {
        recv_sizes.resize(world_size);
    }

    MPI_Gather(&local_size, 1, MPI_INT, recv_sizes.data(), 1, MPI_INT, 0, communicator);

    // compute the displacements from the rank string sizes
    std::vector<int> displacements;
    int total_size = 0;

    if (rank == 0) {
        displacements.resize(world_size);

        for (int i = 0; i < world_size; ++i) {
            displacements[i] = total_size;
            total_size += recv_sizes[i];
        }
    }

    // gather the local yaml strings from all ranks
    std::vector<char> recv_buffer;

    if (rank == 0) {
        recv_buffer.resize(total_size);
    }

    MPI_Gatherv(local_yaml.data(), local_size, MPI_CHAR, recv_buffer.data(), recv_sizes.data(), displacements.data(), MPI_CHAR, 0, communicator);

    // build final yaml string on rank 0
    std::string global_yaml;

    if (rank == 0) {
        for (int r = 0; r < world_size; ++r) {
            global_yaml.append(recv_buffer.data() + displacements[r], recv_sizes[r]);
            global_yaml += '\n';
        }
    }

    return global_yaml;
}
#endif

}  // namespace hws::detail
