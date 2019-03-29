/*******************************************************************************
 * mpi/gather.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file. 
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <mpi.h>

#include "mpi/allreduce.hpp"
#include "mpi/big_type.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dsss::mpi {

template <typename DataType>
inline std::vector<DataType> gather(DataType& send_data, int32_t target,
                                    environment env = environment()) {
  std::vector<DataType> result(env.size(), 0);
  data_type_mapper<DataType> dtm;
  MPI_Gather(
    &send_data,
    1,
    dtm.get_mpi_type(),
    result.data(),
    1,
    dtm.get_mpi_type(),
    target,
    env.communicator());
  return result;
}

template <typename DataType>
inline void gatherv_small(DataType* send_data,
                          int32_t send_count,
                          int32_t target,
                          DataType* recv_buffer,
                          environment env = environment()) {

  std::vector<int32_t> receiving_sizes = gather(send_count, target, env);
  
  std::vector<std::int32_t> receiving_offsets(env.size(), 0);
  for (size_t i = 1; i < receiving_sizes.size(); ++i) {
    receiving_offsets[i] = receiving_offsets[i - 1] + receiving_sizes[i - 1];
  }

  data_type_mapper<DataType> dtm;
  MPI_Gatherv(
    send_data,
    send_count,
    dtm.get_mpi_type(),
    recv_buffer,
    receiving_sizes.data(),
    receiving_offsets.data(),
    dtm.get_mpi_type(),
    target,
    env.communicator());
}

template <typename DataType>
inline void gatherv(DataType* send_data,
                    size_t send_count,
                    int32_t target,
                    DataType* recv_buffer,
                    environment env = environment()) {

  std::vector<size_t> receiving_sizes = gather(send_count, target, env);
  
  std::vector<size_t> receiving_offsets(env.size(), 0);
  if (env.rank() == target) {
    for (size_t i = 1; i < receiving_sizes.size(); ++i) {
      receiving_offsets[i] = receiving_offsets[i - 1] + receiving_sizes[i - 1];
    }
  }
  size_t receiving_size = receiving_sizes.back() + receiving_offsets.back();
  bool local_to_big = receiving_size > env.mpi_max_int();
  bool global_to_big = allreduce_and(local_to_big);
  if (global_to_big) {
    std::vector<MPI_Request> mpi_requests(
      env.rank() == target ? env.size() + 1 : 1);
    
    if (env.rank() == target) {
      for (std::int32_t i = 0; i < env.size(); ++i) {
        auto receive_type = get_big_type<DataType>(receiving_sizes[i]);
        MPI_Irecv(
          recv_buffer + receiving_offsets[i],
          1,
          receive_type,
          i,
          44227,
          env.communicator(),
          &mpi_requests[1 + i]);
      }
    }
    auto send_type = get_big_type<DataType>(send_count);
    MPI_Isend(
      send_data,
      1,
      send_type,
      target,
      44227,
      env.communicator(),
      &mpi_requests[0]);

      MPI_Waitall(2 * env.size(), mpi_requests.data(), MPI_STATUSES_IGNORE);
  } else {
    gatherv_small(send_data, int32_t(send_count), target, recv_buffer, env);
  }
}

} // dsss::mpi

/******************************************************************************/
