/*******************************************************************************
 * tests/mpi/type_mapper_test.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"
#include <mpi.h>

#include "mpi/type_mapper.hpp"
#include "util/macros.hpp"
#include "util/uint_types.hpp"

#include "util/are_same.hpp"

namespace dsss::tests::mpi {

using dsss::mpi::type_mapper;

TEST(type_mapper, factors) {
  ASSERT_EQ(type_mapper<std::int8_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::uint8_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::int16_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::uint16_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::int32_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::uint32_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::int64_t>::factor(), 1);
  ASSERT_EQ(type_mapper<std::uint64_t>::factor(), 1);

  ASSERT_EQ(type_mapper<dsss::uint40>::factor(), 5);
  ASSERT_EQ(type_mapper<dsss::uint48>::factor(), 6);

  struct test_struct {
    std::uint64_t index;
    std::uint8_t  other_index;
  } DSSS_ATTRIBUTE_PACKED; // struct test_struct

  ASSERT_EQ(type_mapper<test_struct>::factor(), 9);

  // ASSERT_FALSE((dsss::util::are_same<std::uint8_t, std::uint64_t>));
}

} // namespace dsss::tests::mpi

/******************************************************************************/
