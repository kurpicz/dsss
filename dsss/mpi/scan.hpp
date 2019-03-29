/*******************************************************************************
 * mpi/scan.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "mpi.h"

#include "mpi/allgather.hpp"
#include "mpi/broadcast.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dsss::mpi {

template <typename DataType>
DataType ex_prefix_sum(DataType local_data, environment const& env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for ex_prefix_sum.");

  DataType result;
  MPI_Exscan(&local_data,
             &result,
             type_mapper<DataType>::factor(),
             type_mapper<DataType>::type(),
             MPI_SUM,
             env.communicator());
  if (env.rank() == 0) { result = DataType(0); }
  return result;
}

template <typename DataType>
DataType rev_ex_prefix_sum(DataType local_data,
                           environment const& env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for ex_prefix_sum.");

  auto all_values = allgather(local_data);
  size_t result = 0;
  for (int32_t i = env.size() - 1; i > env.rank(); --i) {
    result += all_values[i];
  }
  return result;

  // DataType result = prefix_sum(local_data, env);
  // DataType total_size = broadcast(result, env.size() - 1, env);
  // return total_size - result;
}

template <typename DataType>
DataType prefix_sum(DataType local_data, environment const& env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for scan.");

  DataType result;
  MPI_Scan(&local_data,
           &result,
           type_mapper<DataType>::factor(),
           type_mapper<DataType>::type(),
           MPI_SUM,
           env.communicator());
  return result;
}

template <typename DataType>
DataType rev_prefix_sum(DataType local_data, environment const& env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for ex_prefix_sum.");

  DataType result = prefix_sum(local_data, env);
  DataType total_size = broadcast(result, env.size() - 1, env);
  return total_size - (result - local_data);

}

} // namespace dsss::mpi

/******************************************************************************/
