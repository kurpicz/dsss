/*******************************************************************************
 * mpi/scatter.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>
#include <vector>

#include "mpi/big_type.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dsss::mpi {

template <typename DataType>
inline DataType scatter(std::vector<DataType>& send_data, int32_t root,
                        environment env = environment()) {

  data_type_mapper<DataType> dtm;
  DataType result;
  MPI_Scatter(send_data.data(),
              1,
              dtm.get_mpi_type(),
              &result,
              1,
              dtm.get_mpi_type(),
              root,
              env.communicator());
  return result;
}

template <typename DataType>
static inline size_t scatterv_small(DataType* send_data,
                                    std::vector<int32_t>& send_counts,
                                    int32_t root,
                                    DataType* recv_buffer,
                                    environment env = environment()) {

  int32_t recv_count = scatter(send_counts, root, env);
  data_type_mapper<DataType> dtm;

  std::vector<int32_t> offsets(env.size(), 0);
  if (env.rank() == root) {
    for (int32_t i = 1; i < env.size(); ++i) {
      offsets[i] = offsets[i - 1] + send_counts[i - 1];
    }
  }

  MPI_Scatterv(send_data,
               send_counts.data(),
               offsets.data(),
               dtm.get_mpi_type(),
               recv_buffer,
               recv_count,
               dtm.get_mpi_type(),
               root,
               env.communicator());
  return recv_count;
}

template <typename DataType>
static inline size_t scatterv(DataType* send_data,
                              size_t data_size,
                              std::vector<size_t>& send_counts,
                              int32_t root,
                              DataType* recv_buffer,
                              environment env = environment()) {

  if (data_size < env.mpi_max_int()) {
    std::vector<int32_t> casted_counts;
    std::transform(send_counts.begin(), send_counts.end(),
                   std::back_inserter(casted_counts),
                   [](size_t sc) { return static_cast<int32_t>(sc); });
    return scatterv_small(send_data, casted_counts, root, recv_buffer, env);
  } else {
    std::vector<MPI_Request> mpi_requests(env.size() + 1);    
    size_t recv_count = scatter(send_counts, root, env);
    std::vector<size_t> offsets;
    if (env.rank() == root) {
      offsets.resize(env.size());
      for (int32_t i = 1; i < env.size(); ++i) {
        offsets[i] = offsets[i - 1] + send_counts[i - 1];
      }
    }
    if (env.rank() == root) {
      for (int32_t i = 0; i < env.size(); ++i) {
        auto send_type = get_big_type<DataType>(send_counts[i]);
        MPI_Isend(send_data + offsets[i],
                  1,
                  send_type,
                  i,
                  44227,
                  env.communicator(),
                  &mpi_requests[i]);
      }
    }
    auto recv_type = get_big_type<DataType>(recv_count);
    MPI_Irecv(recv_buffer,
              1,
              recv_type,
              root,
              44227,
              env.communicator(),
              &mpi_requests[env.size()]);

    MPI_Waitall(env.size() + 1, mpi_requests.data(), MPI_STATUSES_IGNORE);
    return recv_count;
  }
}

template <typename DataType>
static inline size_t scatter_distibuted(DataType* send_data,
                                        size_t data_size,
                                        int32_t root,
                                        DataType* recv_buffer,
                                        environment env = environment()) {
  
  std::vector<size_t> send_sizes(env.size(), data_size / env.size());
  send_sizes.front() += (data_size % env.size());

  return scatterv(send_data, data_size, send_sizes, root, recv_buffer, env);
}

} // namespace dsss::mpi

/******************************************************************************/
