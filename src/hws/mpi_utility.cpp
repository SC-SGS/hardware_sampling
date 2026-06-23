/**
 * @author Tim Thüring
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/mpi_utility.hpp"

#include <cstddef>        // std::size_t
#include <stdexcept>      // std::runtime_error
#include <string>         // std::string
#include <string_view>    // std::string_view
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

namespace hws::detail {

hostname_comm_info::hostname_comm_info(MPI_Comm comm) {
    int world_rank = 0;
    int world_size = 0;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    // Gather all hostnames
    char name[MPI_MAX_PROCESSOR_NAME];
    int name_len = 0;
    MPI_Get_processor_name(name, &name_len);

    std::vector<int> name_lengths(world_size);
    MPI_Allgather(&name_len, 1, MPI_INT, name_lengths.data(), 1, MPI_INT, comm);

    // Build displacements and total byte count
    std::vector<int> displs(world_size);
    int total = 0;
    for (int i = 0; i < world_size; ++i) {
        displs[i] = total;
        total += name_lengths[i];
    }

    std::vector<char> all_names(total);
    MPI_Allgatherv(name, name_len, MPI_CHAR, all_names.data(), name_lengths.data(), displs.data(), MPI_CHAR, comm);

    // Assign colors locally on every rank
    //
    // All ranks hold identical copies of all_names, name_lengths, and displs,
    // so they can each compute the same deterministic color map independently.

    std::unordered_map<std::string_view, int> host_to_color;
    host_to_color.reserve(world_size);
    std::vector<int> colors(world_size);
    int next_color = 0;
    for (int r = 0; r < world_size; ++r) {
        // get host name of rank r
        std::string_view host(&all_names[displs[r]], static_cast<std::size_t>(name_lengths[r]));

        // try to insert it into the host_to_color map
        auto [it, inserted] = host_to_color.emplace(host, next_color);

        // check if host was new, if yes, increment color
        if (inserted) {
            ++next_color;
        }
        // save color of current rank, either from newly created or existing entry
        colors[r] = it->second;
    }

    // Split communicator
    MPI_Comm_split(comm, colors[world_rank], world_rank, &node_comm);
    MPI_Comm_rank(node_comm, &node_rank);
    MPI_Comm_size(node_comm, &node_size);
}

hostname_comm_info::~hostname_comm_info() {
    if (node_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&node_comm);
    }
}

std::string gather_yaml_strings_mpi(const std::string &local_yaml, MPI_Comm communicator) {
    int initialized = 0;
    MPI_Initialized(&initialized);

    if (!initialized) {
        throw std::runtime_error{"MPI must already be initialized"};
    }

    // MPI rank and world size for identification and communication
    int rank = 0;
    int world_size = 0;
    MPI_Comm_rank(communicator, &rank);
    MPI_Comm_size(communicator, &world_size);

    // gather the size of the yaml string from each rank
    const int local_size = static_cast<int>(local_yaml.size());

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

std::vector<int> owned_local_indices_for_backend(const std::vector<visible_gpu_device> &local_devices, MPI_Comm node_comm) {
    int node_rank = 0;
    int node_size = 0;
    MPI_Comm_rank(node_comm, &node_rank);
    MPI_Comm_size(node_comm, &node_size);

    // Pack physical IDs into a newline-separated string
    std::string packed;
    for (const auto &d : local_devices) {
        packed += d.physical_id;
        packed += '\n';
    }
    const int local_size = static_cast<int>(packed.size());

    // Allgather sizes
    std::vector<int> sizes(node_size);
    MPI_Allgather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, node_comm);

    // Displacements and total length
    std::vector<int> displs(node_size);
    int total = 0;
    for (int r = 0; r < node_size; ++r) {
        displs[r] = total;
        total += sizes[r];
    }

    // Allgatherv packed physical IDs
    std::vector<char> all_data(total);
    MPI_Allgatherv(packed.data(), local_size, MPI_CHAR, all_data.data(), sizes.data(), displs.data(), MPI_CHAR, node_comm);

    // Build owner map: physical_id -> first node_rank that reports it
    std::unordered_map<std::string, int> owner_rank_for_id;
    owner_rank_for_id.reserve(local_devices.size() * 2 + 1);

    for (int r = 0; r < node_size; ++r) {
        if (sizes[r] == 0) {
            continue;
        }

        const char *base = all_data.data() + displs[r];
        const int len = sizes[r];

        int line_start = 0;
        while (line_start < len) {
            int line_end = line_start;
            while (line_end < len && base[line_end] != '\n') {
                ++line_end;
            }
            if (line_end > line_start) {
                const std::string id(base + line_start, base + line_end);  // copy just this ID
                owner_rank_for_id.emplace(id, r);                          // first insertion wins
            }
            line_start = line_end + 1;
        }
    }

    // Decide which local indices we own: those whose physical_id is mapped to node_rank
    std::vector<int> owned_indices;
    owned_indices.reserve(local_devices.size());

    for (const auto &d : local_devices) {
        auto it = owner_rank_for_id.find(d.physical_id);
        if (it != owner_rank_for_id.end() && it->second == node_rank) {
            owned_indices.push_back(d.local_index);
        }
    }

    return owned_indices;
}

}  // namespace hws::detail
