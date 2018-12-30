/*******************************************************************************
 * suffix_sorting/sa_check.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/distribute_data.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"
#include "mpi/shift.hpp"
#include "mpi/sort.hpp"
#include "mpi/zip.hpp"
#include "util/macros.hpp"
#include "util/string.hpp"

namespace dsss::suffix_sorting {

template <typename IndexType>
bool check(std::vector<IndexType>& sa,
  std::vector<dsss::char_type>& text) {

  bool is_correct = true;
  if (sa.size() == 0) { return false; }
  dsss::mpi::environment env;

  struct sa_tuple {
    IndexType rank;
    IndexType sa;
  } DSSS_ATTRIBUTE_PACKED;

  struct rank_triple {
    IndexType rank1;
    IndexType rank2;
    dsss::char_type chr;

    bool operator <(const rank_triple& other) const {
      return std::tie(chr, rank2) < std::tie(other.chr, other.rank2);
    }

    bool operator <=(const rank_triple& other) const {
      return std::tie(chr, rank2) <= std::tie(other.chr, other.rank2);
    }
  } DSSS_ATTRIBUTE_PACKED;

  auto sa_tuples = dsss::mpi::zip_with_index<IndexType>(sa,
    [](IndexType i, IndexType sa_pos) {
      return sa_tuple { i + IndexType(1), sa_pos };
    });

  dsss::mpi::sort(sa_tuples, [](const sa_tuple& a, const sa_tuple& b) {
    return a.sa < b.sa;
  });
  sa_tuples = dsss::mpi::distribute_data(sa_tuples);
  env.barrier();

  size_t local_size = sa_tuples.size();
  size_t offset = dsss::mpi::ex_prefix_sum(local_size);

  bool is_permutation = true;
  for (size_t i = 0; i < local_size; ++i) {
    is_permutation &= (sa_tuples[i].sa == IndexType(i + offset));
  }
  is_correct = dsss::mpi::allreduce_and(is_permutation);
  if (!is_correct) {
    std::cout << "NO PERM" << std::endl;
    return false;
  }

  sa_tuples = dsss::mpi::distribute_data(sa_tuples);
  text = dsss::mpi::distribute_data(text);

  sa_tuple tuple_to_right = dsss::mpi::shift_left(sa_tuples.front());

  if (env.rank() + 1 < env.size()) {
    sa_tuples.emplace_back(tuple_to_right);
  } else {
    sa_tuples.emplace_back(sa_tuple { 0, 0 });
  }

  std::vector<rank_triple> rts;
  for (size_t i = 0; i < local_size; ++i) {
    rts.emplace_back(
      rank_triple { sa_tuples[i].rank, sa_tuples[i + 1].rank, text[i] }
    );
  }

  dsss::mpi::sort(rts, [](const rank_triple& a, const rank_triple& b) {
    return a.rank1 < b.rank1; 
  });

  local_size = rts.size();

  bool is_sorted = true;
  for (size_t i = 0; i < local_size - 1; ++i) {
    is_sorted &= (rts[i] <= rts[i + 1]);
  }

  auto smaller_triple = dsss::mpi::shift_right(rts.back());
  auto larger_triple = dsss::mpi::shift_left(rts.front());

  if (env.rank() > 0) {
    is_sorted &= (smaller_triple < rts.front());
  }
  if (env.rank() + 1 < env.size()) {
    is_sorted &= (rts.back() < larger_triple);
  }

  is_correct = dsss::mpi::allreduce_and(is_sorted);

  if (!is_correct) {
    std::cout << "NOT SORTED" << std::endl;
  }

  return is_correct;
}

} // namespace dsss::suffix_sorting

/******************************************************************************/
