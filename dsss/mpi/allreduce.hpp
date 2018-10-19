/*******************************************************************************
 * mpi/allreduce.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <array>
#include <cmath>
#include <type_traits>
#include <vector>

#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

#include "util/uint_types.hpp"

namespace dsss::mpi {

static inline bool allreduce_and(bool& send_data,
  environment env = environment()) {

  bool receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<bool>::factor(),
    type_mapper<bool>::type(),
    MPI_LAND,
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline DataType allreduce_max(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_max.");
  DataType receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_MAX,
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline DataType allreduce_min(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_min.");
  DataType receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_MIN,
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline DataType allreduce_sum(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_sum.");
  DataType receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_SUM,
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline std::vector<DataType> allreduce_sum(
  std::vector<DataType>& send_data, environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
   "Only arithmetic types are allowed for allreduce_sum.");
  std::vector<DataType> result(send_data.size());
  MPI_Allreduce(
    send_data.data(),
    result.data(),
    send_data.size(),
    type_mapper<DataType>::type(),
    MPI_SUM,
    env.communicator());
  return result;
}

template <typename DataType, size_t Length>
static inline std::array<DataType, Length> allreduce_sum(
  std::array<DataType, Length>& send_data, environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
   "Only arithmetic types are allowed for allreduce_sum.");
  std::array<DataType, Length> result;
  MPI_Allreduce(
    send_data.data(),
    result.data(),
    Length * type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_SUM,
    env.communicator());
  return result;
}

template <size_t Length>
static inline std::array<dsss::uint40, Length> allreduce_sum(
  std::array<dsss::uint40, Length>& send_data, environment env = environment()) {

  std::array<size_t, Length> tmp;
  for (size_t i = 0; i < Length; ++i) { tmp[i] = send_data[i]; }
  tmp = allreduce_sum(tmp, env);
  std::array<dsss::uint40, Length> result;
  for (size_t i = 0; i < Length; ++i) { result[i] = tmp[i]; }
  return result;
}

template <typename DataType>
static inline DataType allreduce_avg(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_avg.");
  DataType receive_data = allreduce_sum(send_data);
  return receive_data / env.size();
}

// Sample standard deviation
template <typename DataType>
static inline DataType allreduce_ssd(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_ssd.");
  DataType receive_data = allreduce_avg(send_data);
  DataType tmp = std::pow(send_data - receive_data, 2);
  receive_data = allreduce_sum(tmp);
  return std::sqrt(receive_data / env.size());
}

} // namespace dsss::mpi

/******************************************************************************/
