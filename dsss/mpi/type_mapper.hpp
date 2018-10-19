/*******************************************************************************
 * dsss/mpi/type_mapper.hpp
 *
 * Copyright (c) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <mpi.h>
#include <tuple>
#include <type_traits>

#include "util/are_same.hpp"

namespace dsss::mpi {

template <typename CXXType>
struct type_mapper {
  /* Notes, source: http://en.cppreference.com/w/cpp/types/is_trivially_copyable
   * Objects of trivially-copyable types are the only C++ objects that may be
   * safely copied with std::memcpy or serialized to/from binary files with
   * std::ofstream::write()/std::ifstream::read(). In general, a trivially
   * copyable type is any type for which the underlying bytes can be copied to
   * an array of char or unsigned char and into a new object of the same type,
   * and the resulting object would have the same value as the original.
  */
  static_assert(std::is_trivially_copyable<CXXType>::value,
    "Only is_trivially copyable types can be mapped.");

  static constexpr MPI_Datatype type() { return MPI_BYTE; }
  static constexpr std::uint64_t factor() { return sizeof(CXXType); }
};

#define DSSS_INTEGRAL_DATATYPE_MAPPER(integral_type, mpi_type) \
template <>                                                    \
struct type_mapper<integral_type> {                            \
  static constexpr MPI_Datatype type() { return mpi_type; }    \
  static constexpr std::uint64_t factor() { return 1; }        \
};                                                             \

DSSS_INTEGRAL_DATATYPE_MAPPER(std::int8_t, MPI_CHAR)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::uint8_t, MPI_BYTE)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::int16_t, MPI_SHORT)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::uint16_t, MPI_UNSIGNED_SHORT)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::int32_t, MPI_INT)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::uint32_t, MPI_UNSIGNED)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::int64_t, MPI_LONG_LONG_INT)
DSSS_INTEGRAL_DATATYPE_MAPPER(std::uint64_t, MPI_UNSIGNED_LONG_LONG)
DSSS_INTEGRAL_DATATYPE_MAPPER(double, MPI_DOUBLE)

#undef DSSS_INTEGRAL_DATATYPE_MAPPER

template <typename... CXXTypes>
struct type_mapper<std::tuple<CXXTypes ...>> {
  static constexpr MPI_Datatype type() { return MPI_BYTE; }
  static constexpr std::uint64_t factor() { return (sizeof(CXXTypes) + ...); }
};

template <typename CXXType>
class data_type_mapper {

public:
  data_type_mapper() : custom_(type_mapper<CXXType>::factor() != 1) {
    if (custom_) {
      MPI_Type_contiguous(type_mapper<CXXType>::factor(),
                          MPI_BYTE,
                          &mpi_datatype_);
      MPI_Type_commit(&mpi_datatype_);
    } else { mpi_datatype_ = type_mapper<CXXType>::type(); }
  }

  ~data_type_mapper() {
    if (custom_) { MPI_Type_free(&mpi_datatype_); }
  }

  MPI_Datatype get_mpi_type() const { return mpi_datatype_; }

private:
  bool custom_;
  MPI_Datatype mpi_datatype_;
}; // class datatype

} // namespace dsss::mpi

/******************************************************************************/
