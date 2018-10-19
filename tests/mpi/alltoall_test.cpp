/*******************************************************************************
 * tests/mpi/alltoall_test.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"
#include <mpi.h>
#include <tuple>

#include "mpi/alltoall.hpp"
#include "mpi/environment.hpp"
#include "util/indexed_string_set.hpp"
#include "util/macros.hpp"
#include "util/string_set.hpp"

namespace dsss::tests::mpi {

TEST(alltoall, correctness) {
  dsss::mpi::environment env;

  std::vector<std::size_t> send_data;
  for (std::int64_t i = 0; i < 10 * env.size(); ++i) {
    send_data.emplace_back(env.rank());
  }
  std::vector<std::size_t> receive_data = dsss::mpi::alltoall(send_data);

  for (std::int64_t i = 0; i < env.size(); ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      ASSERT_EQ(i, receive_data[(10 * i) + j]);
    }
  }
}

TEST(alltoallv, same_size) {
  dsss::mpi::environment env;

  std::vector<std::size_t> send_data;
  for (std::int64_t i = 0; i < 10 * env.size(); ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      send_data.emplace_back(env.rank());
    }
  }
  std::vector<std::size_t> send_counts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_counts.emplace_back(10);
  }
  auto result = dsss::mpi::alltoallv(send_data, send_counts);

  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::size_t i = 0; i < 10; ++i) {
      ASSERT_EQ(result[(rank * 10) + i], rank);
    }
  }
}

TEST(alltoallv, tuple) {
  dsss::mpi::environment env;

  std::vector<std::tuple<std::size_t, std::size_t>> send_data;

  for (std::int64_t i = 0; i < 10 * env.size(); ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      send_data.emplace_back(
        std::make_tuple(static_cast<std::size_t>(env.rank()),
          static_cast<std::int32_t>(env.rank())));
    }
  }
  std::vector<std::size_t> send_counts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_counts.emplace_back(10);
  }
  auto result = dsss::mpi::alltoallv(send_data, send_counts);

  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::size_t i = 0; i < 10; ++i) {
      ASSERT_EQ(std::get<0>(result[(rank * 10) + i]), rank);
      ASSERT_EQ(std::get<1>(result[(rank * 10) + i]), rank);
    }
  }
}

TEST(alltoallv_strings, same_sizes) {
  dsss::mpi::environment env;
  std::vector<dsss::char_type> raw_send_data;

  for (std::int64_t i = 0; i < 10 * env.size(); ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      raw_send_data.emplace_back((env.rank() % 128) + 1);
    }
    raw_send_data.emplace_back(0);
  }
  dsss::string_set send_data(std::move(raw_send_data));
  std::vector<std::size_t> send_cnts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_cnts.emplace_back(10);
  }

  dsss::string_set result = dsss::mpi::alltoallv_strings(send_data, send_cnts);

  ASSERT_EQ(result.size(), 10 * env.size());
  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::size_t str = 0; str < 10; ++str) {
      for (std::size_t pos = 0; pos < 10; ++pos) {
        ASSERT_EQ(result[(10 * rank) + str][pos], (rank % 128) + 1);
      }
    }
  }
}

TEST(alltoallv_strings, different_sizes) {
  dsss::mpi::environment env;
  std::vector<dsss::char_type> raw_send_data;

  for (std::int64_t i = 0; i < (env.rank() + 1) * env.size(); ++i) {
    for (std::int64_t j = 0; j < env.rank() + 10; ++j) {
      raw_send_data.emplace_back((env.rank() % 128) + 1);
    }
    raw_send_data.emplace_back(0);
  }
  dsss::string_set send_data(std::move(raw_send_data));
  std::vector<std::size_t> send_cnts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_cnts.emplace_back(env.rank() + 1);
  }

  dsss::string_set result = dsss::mpi::alltoallv_strings(send_data, send_cnts);

  std::size_t nr_rec_strings = 0;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    nr_rec_strings += (i + 1);
  } 
  ASSERT_EQ(result.size(), nr_rec_strings);

  std::size_t string_offset = 0;
  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::int64_t str = 0; str < rank + 1; ++str) {
      for (std::int64_t pos = 0; pos < rank + 10; ++pos) {
        ASSERT_EQ(result[string_offset + str][pos], (rank % 128) + 1);
      }
    }
    string_offset += (rank + 1);
  }
}

TEST(alltoallv_indexed_strings, same_sizes) {
  dsss::mpi::environment env;
  std::vector<dsss::char_type> raw_send_data;

  std::vector<std::size_t> indices;
  for (std::int64_t i = 0; i < 10 * env.size(); ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      raw_send_data.emplace_back((env.rank() % 128) + 1);
    }
    raw_send_data.emplace_back(0);
    indices.emplace_back(i);
  }
  dsss::indexed_string_set<std::size_t> send_data(
    std::move(raw_send_data), std::move(indices));
  std::vector<std::size_t> send_cnts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_cnts.emplace_back(10);
  }

  auto result = dsss::mpi::alltoallv_indexed_strings(send_data, send_cnts);

  ASSERT_EQ(result.size(), 10 * env.size());
  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::size_t str = 0; str < 10; ++str) {
      for (std::size_t pos = 0; pos < 10; ++pos) {
        ASSERT_EQ(result[(10 * rank) + str].string[pos], (rank % 128) + 1);
      }
      ASSERT_EQ(result[(10 * rank) + str].index, (10 * env.rank()) + str);
    }
  }
}

TEST(alltoallv_indexed_strings, different_sizes) {
  dsss::mpi::environment env;
  std::vector<dsss::char_type> raw_send_data;

  std::vector<std::size_t> indices;
  for (std::int64_t i = 0; i < (env.rank() + 1) * env.size(); ++i) {
    for (std::int64_t j = 0; j < env.rank() + 10; ++j) {
      raw_send_data.emplace_back((env.rank() % 128) + 1);
    }
    raw_send_data.emplace_back(0);
    indices.emplace_back(i);
  }
  dsss::indexed_string_set<std::size_t> send_data(std::move(raw_send_data),
    std::move(indices));
  std::vector<std::size_t> send_cnts;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    send_cnts.emplace_back(env.rank() + 1);
  }

  auto result = dsss::mpi::alltoallv_indexed_strings(send_data, send_cnts);

  std::size_t nr_rec_strings = 0;
  for (std::int64_t i = 0; i < env.size(); ++i) {
    nr_rec_strings += (i + 1);
  } 
  ASSERT_EQ(result.size(), nr_rec_strings);

  std::size_t string_offset = 0;
  for (std::int64_t rank = 0; rank < env.size(); ++rank) {
    for (std::int64_t str = 0; str < rank + 1; ++str) {
      for (std::int64_t pos = 0; pos < rank + 10; ++pos) {
        ASSERT_EQ(result[string_offset + str].string[pos], (rank % 128) + 1);
      }
      ASSERT_EQ(result[string_offset + str].index,
        (env.rank() * (rank + 1)) + str);
    }
    string_offset += (rank + 1);
  }
}

} // namespace dsss::tests::mpi

/******************************************************************************/
