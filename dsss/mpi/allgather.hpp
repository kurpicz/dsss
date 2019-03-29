/*******************************************************************************
 * mpi/allgather.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <mpi.h>
#include <vector>

#include "mpi/alltoall.hpp"
#include "mpi/big_type.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"
#include "util/string_set.hpp"

namespace dsss::mpi {

template <typename DataType>
inline std::vector<DataType> allgather(DataType& send_data,
                                       environment env = environment()) {

  data_type_mapper<DataType> dtm;
  std::vector<DataType> receive_data(env.size());
  MPI_Allgather(
    &send_data,
    1,
    dtm.get_mpi_type(),
    receive_data.data(),
    1,
    dtm.get_mpi_type(),
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline std::vector<DataType> allgatherv_small(
  std::vector<DataType>& send_data, environment env = environment()) {

  int32_t local_size = send_data.size();
  std::vector<int32_t> receiving_sizes = allgather(local_size);

  std::vector<int32_t> receiving_offsets(env.size(), 0);
  for (size_t i = 1; i < receiving_sizes.size(); ++i) {
    receiving_offsets[i] = receiving_offsets[i - 1] + receiving_sizes[i - 1];
  }

  std::vector<DataType> receiving_data(
    receiving_sizes.back() + receiving_offsets.back());

  data_type_mapper<DataType> dtm;
  MPI_Allgatherv(
    send_data.data(),
    local_size,
    dtm.get_mpi_type(),
    receiving_data.data(),
    receiving_sizes.data(),
    receiving_offsets.data(),
    dtm.get_mpi_type(),
    env.communicator());

  return receiving_data;
}

template <typename DataType>
static inline std::vector<DataType> allgatherv(
  std::vector<DataType>& send_data, environment env = environment()) {

  size_t local_size = send_data.size();
  std::vector<size_t> receiving_sizes = allgather(local_size);

  std::vector<size_t> receiving_offsets(env.size(), 0);
  for (size_t i = 1; i < receiving_sizes.size(); ++i) {
    receiving_offsets[i] = receiving_offsets[i - 1] + receiving_sizes[i - 1];
  }

  if (receiving_sizes.back() + receiving_offsets.back() < env.mpi_max_int()) {
    return allgatherv_small(send_data);
  } else {
    std::vector<MPI_Request> mpi_requests(2 * env.size());
    std::vector<DataType> receiving_data(
      receiving_sizes.back() + receiving_offsets.back());

    for (int32_t i = 0; i < env.size(); ++i) {
      auto receive_type = get_big_type<DataType>(receiving_sizes[i]);
      MPI_Irecv(
        receiving_data.data() + receiving_offsets[i],
        1,
        receive_type,
        i,
        44227,
        env.communicator(),
        &mpi_requests[i]);
    }
    auto send_type = get_big_type<DataType>(local_size);
    for (int32_t i = env.rank(); i < env.rank() + env.size(); ++i) {
      int32_t target = i % env.size();
      MPI_Isend(
        send_data.data(),
        1,
        send_type,
        target,
        44227,
        env.communicator(),
        &mpi_requests[env.size() + target]);
    }
    MPI_Waitall(2 * env.size(), mpi_requests.data(), MPI_STATUSES_IGNORE);
    return receiving_data;
  }
}

static inline dsss::string_set allgather_strings(
  std::vector<dsss::char_type>& raw_string_data,
  environment env = environment()) {

  auto receiving_data = allgatherv(raw_string_data, env);
  return dsss::string_set(std::move(receiving_data));
}

} // namespace dsss::mpi

/******************************************************************************/
