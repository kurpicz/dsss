/*******************************************************************************
 * mpi/shift.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <mpi.h>

#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"
#include "util/string.hpp"

namespace dsss::mpi {

template <typename DataType>
static inline DataType shift_left(DataType& send_data,
  environment env = environment()) {

  if (env.size() == 1) {
    return send_data;
  }

  std::int32_t destination = env.size() - 1;
  if (env.rank() > 0) {
    destination = env.rank() - 1;
  }
  std::int32_t source = 0;
  if (env.rank() + 1 < env.size()) {
    source = env.rank() + 1;
  }

  data_type_mapper<DataType> dtm;
  DataType receive_data;
  MPI_Sendrecv(&send_data,
               1,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               &receive_data,
               1,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

template <typename DataType>
static inline DataType shift_right(DataType& send_data,
  environment env = environment()) {

  if (env.size() == 1) {
    return send_data;
  }

  std::int32_t destination = 0;
  if (env.rank() + 1 < env.size()) {
    destination = env.rank() + 1;
  }
  std::int32_t source = env.size() - 1;
  if (env.rank() > 0) {
    source = env.rank() - 1;
  }

  data_type_mapper<DataType> dtm;
  DataType receive_data;
  MPI_Sendrecv(&send_data,
               1,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               &receive_data,
               1,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

template <typename DataType>
static inline std::vector<DataType> shift_left(DataType* send_data,
  std::size_t count, environment env = environment()) {

  std::int32_t destination = env.size() - 1;
  if (env.rank() > 0) {
    destination = env.rank() - 1;
  }
  std::int32_t source = 0;
  if (env.rank() + 1 < env.size()) {
    source = env.rank() + 1;
  }

  std::size_t receive_count = shift_left(count, env);

  std::vector<DataType> receive_data(receive_count);
  data_type_mapper<DataType> dtm;
  MPI_Sendrecv(send_data,
               count,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               receive_data.data(),
               receive_count,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

template <typename DataType>
static inline std::vector<DataType> shift_right(DataType* send_data,
  std::size_t count, environment env = environment()) {

  std::int32_t destination = 0;
  if (env.rank() + 1 < env.size()) {
    destination = env.rank() + 1;
  }
  std::int32_t source = env.size() - 1;
  if (env.rank() > 0) {
    source = env.rank() - 1;
  }

  std::size_t receive_count = shift_right(count, env);

  std::vector<DataType> receive_data(count);
  data_type_mapper<DataType> dtm;
  MPI_Sendrecv(send_data,
               count,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               receive_data.data(),
               receive_count,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

static inline std::vector<dsss::char_type> shift_string_left(
  dsss::string send_data, environment env = environment()) {

  std::size_t send_length = dsss::string_length(send_data) + 1;
  std::vector<char_type> real_send_data;
  std::copy_n(send_data, send_length, std::back_inserter(real_send_data));

  if (env.size() == 1) {
    return real_send_data;
  }

  std::size_t receive_length = shift_left(send_length, env);

  std::int32_t destination = env.size() - 1;
  if (env.rank() > 0) {
    destination = env.rank() - 1;
  }
  std::int32_t source = 0;
  if (env.rank() + 1 < env.size()) {
    source = env.rank() + 1;
  }

  std::vector<dsss::char_type> receive_data(receive_length);
  data_type_mapper<dsss::char_type> dtm;
  MPI_Sendrecv(real_send_data.data(),
               send_length,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               receive_data.data(),
               receive_length,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

static inline std::vector<dsss::char_type> shift_string_right(
  dsss::string send_data, environment env = environment()) {

  std::size_t send_length = dsss::string_length(send_data) + 1;
  std::vector<char_type> real_send_data;
  std::copy_n(send_data, send_length, std::back_inserter(real_send_data));

  if (env.size() == 1) {
    return real_send_data;
  }

  std::size_t receive_length = shift_right(send_length, env);

  std::int32_t destination = 0;
  if (env.rank() + 1 < env.size()) {
    destination = env.rank() + 1;
  }
  std::int32_t source = env.size() - 1;
  if (env.rank() > 0) {
    source = env.rank() - 1;
  }

  std::vector<dsss::char_type> receive_data(receive_length);
  data_type_mapper<dsss::char_type> dtm;
  MPI_Sendrecv(real_send_data.data(),
               send_length,
               dtm.get_mpi_type(),
               destination,
               0, // chose arbitrary tag
               receive_data.data(),
               receive_length,
               dtm.get_mpi_type(),
               source,
               MPI_ANY_TAG,
               env.communicator(),
               MPI_STATUS_IGNORE);
  return receive_data;
}

} // namespace dsss::mpi

/******************************************************************************/
