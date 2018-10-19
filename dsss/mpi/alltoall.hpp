/*******************************************************************************
 * mpi/alltoall.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <mpi.h>
#include <numeric>
#include <iterator>
#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/big_type.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"
#include "mpi/scan.hpp"
#include "util/indexed_string_set.hpp"
#include "util/string.hpp"
#include "util/string_set.hpp"


namespace dsss::mpi {

static constexpr bool debug_alltoall = false;

template <typename DataType>
inline std::vector<DataType> alltoall(std::vector<DataType>& send_data,
  environment env = environment()) {
  std::vector<DataType> receive_data(send_data.size(), 0);
  data_type_mapper<DataType> dtm;
  MPI_Alltoall(send_data.data(),
               send_data.size() / env.size(),
               dtm.get_mpi_type(),
               receive_data.data(),
               send_data.size() / env.size(),
               dtm.get_mpi_type(),
               env.communicator());
  return receive_data;
}

template <typename DataType>
inline std::vector<DataType> alltoallv_small(
  std::vector<DataType>& send_data, std::vector<size_t>& send_counts,
  environment env = environment()) {

  std::vector<int32_t> real_send_counts(send_counts.size());
  for (size_t i = 0; i < send_counts.size(); ++i) {
    real_send_counts[i] = static_cast<int32_t>(send_counts[i]);
  }
  std::vector<int32_t> receive_counts = alltoall(real_send_counts, env);

  std::vector<int32_t> send_displacements(real_send_counts.size(), 0);
  std::vector<int32_t> receive_displacements(real_send_counts.size(), 0);
  for (size_t i = 1; i < real_send_counts.size(); ++i) {
    send_displacements[i] = send_displacements[i - 1] + real_send_counts[i - 1];
    receive_displacements[i] = receive_displacements[i - 1] +
      receive_counts[i - 1];
  }
  std::vector<DataType> receive_data(
    receive_counts.back() + receive_displacements.back());

  if constexpr (debug_alltoall) {
    for (int32_t i = 0; i < env.size(); ++i) {
      if (i == env.rank()) {
        std::cout << i << ": send_counts.size() " << send_counts.size()
                  << std::endl;
        std::cout << i << ": send counts: ";
        for (const auto sc : real_send_counts) { std::cout << sc << ", "; }
        std::cout << std::endl << "receive counts: ";

        for (const auto rc : receive_counts) { std::cout << rc << ", "; }
        std::cout << std::endl;
      }
      env.barrier();
    }
  }

  data_type_mapper<DataType> dtm;
  MPI_Alltoallv(send_data.data(),
                real_send_counts.data(),
                send_displacements.data(),
                dtm.get_mpi_type(),
                receive_data.data(),
                receive_counts.data(),
                receive_displacements.data(),
                dtm.get_mpi_type(),
                env.communicator());
  return receive_data;
}

template <typename DataType>
inline std::vector<DataType> alltoallv(std::vector<DataType>& send_data,
    std::vector<size_t>& send_counts, environment env = environment()) {

  size_t local_send_count = std::accumulate(
    send_counts.begin(), send_counts.end(), 0);

  std::vector<size_t> receive_counts = alltoall(send_counts, env);
  size_t local_receive_count = std::accumulate(
    receive_counts.begin(), receive_counts.end(), 0);

  size_t local_max = std::max(local_send_count, local_receive_count);
  size_t global_max = allreduce_max(local_max, env);

  if (global_max < env.mpi_max_int()) {
    return alltoallv_small(send_data, send_counts, env);
  } else {
    std::vector<size_t> send_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      send_displacements[i] = send_displacements[i - 1] + send_counts[i - 1];
    }
    std::vector<size_t> receive_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      receive_displacements[i] =
        receive_displacements[i - 1] + receive_counts[i - 1];
    }

    std::vector<MPI_Request> mpi_request(2 * env.size());
    std::vector<DataType> receive_data(receive_displacements.back() +
      receive_counts.back());
    for (int32_t i = 0; i < env.size(); ++i) {
      // start with self send/recv
      auto source = (env.rank() + (env.size() - i)) % env.size();
      auto receive_type = get_big_type<DataType>(receive_counts[source]);
      MPI_Irecv(receive_data.data() + receive_displacements[source],
                1,
                receive_type,
                source,
                44227,
                env.communicator(),
                &mpi_request[source]);
    }
    // dispatch sends
    for (int32_t i = 0; i < env.size(); ++i) {
      auto target = (env.rank() + i) % env.size();
      auto send_type = get_big_type<DataType>(send_counts[target]);
      MPI_Isend(send_data.data() + send_displacements[target],
                1,
                send_type,
                target,
                44227,
                env.communicator(),
                &mpi_request[env.size() + target]);
    }
    MPI_Waitall(2 * env.size(), mpi_request.data(), MPI_STATUSES_IGNORE);
    return receive_data;
  }
}

inline std::vector<dsss::char_type> alltoallv_strings(
  dsss::string_set& send_data, const std::vector<size_t>& send_counts,
  environment env = environment()) {

  const size_t size = send_counts.size();
  std::vector<size_t> send_counts_char(size, 0);
  std::vector<dsss::char_type> send_buffer;
  send_buffer.reserve(send_data.data_container().size());
  // Determine the number of character that must be sended to each node
  for (size_t interval = 0, offset = 0; interval < size; ++interval) {
    // We cannot be sure that the pointers are ordered anymore (i.e., the memory
    // positions are not monotone Increasing)
    for (size_t j = offset; j < send_counts[interval] + offset; ++j) {
      const size_t string_length = dsss::string_length(send_data[j]) + 1;
      send_counts_char[interval] += string_length;
      std::copy_n(send_data[j], string_length, std::back_inserter(send_buffer));
    }
    offset += send_counts[interval];
  }

  if constexpr (debug_alltoall) {
    const size_t total_chars_sent = std::accumulate(
      send_counts_char.begin(), send_counts_char.end(), 0);
    const size_t total_chars_count = send_data.data_container().size();

    for (int32_t rank = 0; rank < env.size(); ++rank) {
      if (env.rank() == rank) {
        std::cout << rank << ": total_chars_sent " << total_chars_sent
                  << ", total_chars_count: " << total_chars_count << std::endl;
      }
      env.barrier();
    }
  }
  return alltoallv(send_buffer, send_counts_char, env);
}

template <typename IndexType>
inline dsss::indexed_string_set<IndexType> alltoallv_indexed_strings(
  dsss::indexed_string_set<IndexType>& send_data,
  std::vector<size_t>& send_counts_strings,
  environment env = environment()) {

  // Send the strings
  assert(send_counts_strings.size() == env.size());
  const size_t size = send_counts_strings.size();
  std::vector<dsss::char_type> real_send_data;
  std::vector<IndexType> index_send_data;
  std::vector<size_t> send_counts_char(size, 0);
  std::vector<size_t> send_displacements(size, 0);
  // Determine the number of character that must be sended to each node
  for (size_t scs_pos = 0, string_pos = 0; scs_pos < size; ++scs_pos) {
    for (size_t i = 0; i < send_counts_strings[scs_pos]; ++i) {
      index_send_data.emplace_back(send_data[string_pos].index);
      // The "+1" is there to also send the terminating 0
      const size_t string_length =
        dsss::string_length(send_data[string_pos].string) + 1;
      send_counts_char[scs_pos] += string_length;
      std::copy_n(send_data[string_pos].string, string_length,
        std::back_inserter(real_send_data));
      ++string_pos;
    }
  }

  std::vector<dsss::char_type> receive_data = alltoallv(real_send_data,
                                                        send_counts_char,
                                                        env);
  std::vector<IndexType> receive_data_indices = alltoallv(index_send_data,
                                                          send_counts_strings,
                                                          env);
  return dsss::indexed_string_set<IndexType>(std::move(receive_data),
    std::move(receive_data_indices));
}

} // namespace dsss::mpi

/******************************************************************************/
