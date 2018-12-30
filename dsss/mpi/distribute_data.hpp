/*******************************************************************************
 * mpi/distribute_data.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

#include "mpi/alltoall.hpp"
#include "mpi/allreduce.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"

#include "util/drop.hpp"

namespace dsss::mpi {

template <typename DataType>
std::vector<DataType> distribute_data(std::vector<DataType>& local_data,
  environment env = environment()) {

  size_t cur_local_size = local_data.size();
  size_t total_size = allreduce_sum(cur_local_size);
  size_t local_size = std::max<size_t>(1, total_size / env.size());

  size_t local_data_size = local_data.size();
  size_t preceding_size = ex_prefix_sum(local_data_size, env);

  auto get_target_rank = [&](const size_t pos) {
    return std::min<int32_t>(env.size() - 1, pos / local_size);
  };

  std::vector<size_t> send_counts(env.size(), 0);
  for (auto cur_rank = get_target_rank(preceding_size);
    local_data_size > 0 && cur_rank < env.size(); ++cur_rank) {
    const size_t to_send = std::min(
      ((cur_rank + 1) * local_size) - preceding_size, local_data_size);
    send_counts[cur_rank] = to_send;
    local_data_size -= to_send;
    preceding_size += to_send;
  }
  send_counts.back() += local_data_size;

  std::vector<DataType> result = alltoallv(local_data, send_counts, env);
  return result;
}

template <typename DataType>
std::vector<DataType> fill_up_data(std::vector<DataType>& send_data,
  size_t local_size, const size_t filled_positions,
  environment env = environment()) {

  size_t local_data_size = send_data.size();
  size_t preceding_size = ex_prefix_sum(local_data_size, env) + filled_positions;

  auto get_target_rank = [&](const size_t pos) {
    return std::min<int32_t>(env.size() - 1, pos / local_size);
  };

  std::vector<size_t> send_counts(env.size(), 0);
  for (auto cur_rank = get_target_rank(preceding_size);
    local_data_size > 0 && cur_rank < env.size(); ++cur_rank) {
    const size_t to_send = std::min(
      ((cur_rank + 1) * local_size) - preceding_size, local_data_size);
    send_counts[cur_rank] = to_send;
    local_data_size -= to_send;
    preceding_size += to_send;
  }

  std::vector<DataType> result = alltoallv(send_data, send_counts, env);
  return result;
}

} // namespace dsss::mpi

/******************************************************************************/
