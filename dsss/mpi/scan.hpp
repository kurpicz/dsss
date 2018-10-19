/*******************************************************************************
 * mpi/scan.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "mpi.h"

#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dsss::mpi {

template <typename DataType>
DataType ex_prefix_sum(DataType& local_data, environment env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for ex_prefix_sum.");
  DataType result;
  MPI_Exscan(
    &local_data,
    &result,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_SUM,
    env.communicator());
  if (env.rank() == 0) { result = DataType(0); }
  return result;
}

template <typename DataType>
DataType prefix_sum(DataType& local_data, environment env = environment()) {
  static_assert(std::numeric_limits<DataType>::is_integer,
    "Only integers are allowed for scan.");
  DataType result;
  MPI_Scan(
    &local_data,
    &result,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_SUM,
    env.communicator());
  return result;
}

} // namespace dsss::mpi

/******************************************************************************/
