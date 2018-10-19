/*******************************************************************************
 * string_sorting/distributed/sample_sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>

#include <tlx/container/loser_tree.hpp>

#include "mpi/allgather.hpp"
#include "mpi/alltoall.hpp"
#include "mpi/environment.hpp"

#include "util/indexed_string_set.hpp"
#include "util/string.hpp"
#include "util/string_set.hpp"

namespace dsss::sample_sort {

static constexpr bool debug = false;
static constexpr bool print_interval_details = debug && true;

template <typename IndexType,
          void LocalIdxSorter(dsss::indexed_string<IndexType>*, std::size_t),
          void LocalSorter(dsss::string*, std::size_t)>
static inline void sample_sort(
  dsss::indexed_string_set<IndexType>& local_string_set,
  dsss::mpi::environment env = dsss::mpi::environment()) {

  std::size_t local_n = local_string_set.size();
  auto* local_strings = local_string_set.strings();
  LocalIdxSorter(local_strings, local_n);

  // There is only one PE, hence there is no need for distributed sorting 
  if (env.size() == 1) {
    return;
  }

  auto nr_splitters = std::min<std::size_t>(env.size() - 1, local_n);
  auto splitter_dist = local_n / (nr_splitters + 1);
  std::vector<dsss::char_type> raw_splitters;
  for (std::size_t i = 1; i <= nr_splitters; ++i) {
    const auto splitter = local_strings[i * splitter_dist].string;
    std::copy_n(splitter, dsss::string_length(splitter) + 1,
      std::back_inserter(raw_splitters));
  }

  // Gather all splitters and sort them to determine the final splitters

  dsss::string_set splitters =
    dsss::mpi::allgather_strings(raw_splitters, env);

  LocalSorter(splitters.strings(), splitters.size());

  nr_splitters = std::min<std::size_t>(env.size() - 1, splitters.size());
  splitter_dist = splitters.size() / (nr_splitters + 1);
  raw_splitters.clear();
  for (std::size_t i = 1; i <= nr_splitters; ++i) {
    const auto splitter = splitters[i * splitter_dist];
    std::copy_n(splitter, dsss::string_length(splitter) + 1,
      std::back_inserter(raw_splitters));
  }
  splitters = string_set(std::move(raw_splitters));

  // Now we need to split the local strings using the splitters
  // The size is given by the NUMBER of strings, not their lengths. The number
  // of characters that must be send to other PEs is computed in the alltoall-
  // function.
  std::vector<std::size_t> interval_sizes;
  std::size_t element_pos = 0;
  splitter_dist = local_n / (nr_splitters + 1);
  for (std::size_t i = 0; i < splitters.size(); ++i) {
    element_pos = (i + 1) * splitter_dist;
    while(element_pos > 1 && !dsss::string_smaller_eq(
      local_strings[element_pos].string, splitters[i])) { --element_pos; }
    while (element_pos < local_n && dsss::string_smaller_eq(
      local_strings[element_pos].string, splitters[i])) { ++element_pos; }
    interval_sizes.emplace_back(element_pos);
  }
  interval_sizes.emplace_back(local_n);
  for (std::size_t i = interval_sizes.size() - 1; i > 0; --i) {
    interval_sizes[i] -= interval_sizes[i - 1];
  }

  std::vector<std::size_t> receiving_sizes = dsss::mpi::alltoall(interval_sizes);
  if constexpr (print_interval_details) {
    for (std::int32_t rank = 0; rank < env.size(); ++rank) {
      if (env.rank() == rank) {
        std::size_t total_size = 0;
        std::cout << "### Sending interval sizes on PE " << rank << std::endl;
        for (const auto is : interval_sizes) {
          total_size += is;
          std::cout << is << ", ";
        }
        std::cout << "Total size: " << total_size << std::endl;
      }
      env.barrier();
    }
    for (std::int32_t rank = 0; rank < env.size(); ++rank) {
      if (env.rank() == rank) {
        std::size_t total_size = 0;
        std::cout << "### Receiving interval sizes on PE " << rank << std::endl;
        for (const auto is : receiving_sizes) {
          total_size += is;
          std::cout << is << ", ";
        }
        std::cout << "Total size: " << total_size << std::endl;
      }
      env.barrier();
    }
    if (env.rank() == 0) { std::cout << std::endl; }
  }

  local_string_set = dsss::mpi::alltoallv_indexed_strings<IndexType>(
    local_string_set, interval_sizes);

  std::vector<decltype(local_string_set.cbegin())> string_it(
    env.size(), local_string_set.cbegin());
  std::vector<decltype(local_string_set.cbegin())> end_it(
    env.size(), local_string_set.cbegin() + receiving_sizes[0]);

  for (std::int32_t i = 1; i < env.size(); ++i) {
    string_it[i] = string_it[i - 1] + receiving_sizes[i - 1];
    end_it[i] = end_it[i - 1] + receiving_sizes[i];
  }

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Sort received strings" << std::endl;
    }
    env.barrier();
  }

  struct idx_string_compare {
    bool operator ()(const dsss::indexed_string<IndexType>& a,
      const dsss::indexed_string<IndexType>& b) {
      return (dsss::string_cmp(a, b) < 0);
    }
  }; // struct string_compare

  tlx::LoserTreeCopy<false, dsss::indexed_string<IndexType>, idx_string_compare>
    lt(env.size());

  std::size_t filled_sources = 0;
  for (std::int32_t i = 0; i < env.size(); ++i) {
    if (string_it[i] == end_it[i]) { lt.insert_start(nullptr, i, true); }
    else {
      lt.insert_start(&*string_it[i], i, false);
      ++filled_sources;
    }
  }

  lt.init();

  std::vector<dsss::indexed_string<IndexType>> result;
  result.reserve(local_string_set.size());
  while (filled_sources) {
    std::int32_t source = lt.min_source();
    result.push_back(*string_it[source]);
    ++string_it[source];
    if (string_it[source] != end_it[source]) {
      lt.delete_min_insert(&*string_it[source], false);
    } else {
      lt.delete_min_insert(nullptr, true);
      --filled_sources;
    }
  }
  local_string_set.update(std::move(result));

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Finished" << std::endl;
    }
    env.barrier();
  }
}

template <void LocalSorter(dsss::string*, std::size_t)>
static inline void sample_sort(dsss::string_set& local_string_set,
  dsss::mpi::environment env = dsss::mpi::environment()) {

  std::size_t local_n = local_string_set.size();
  dsss::string* local_strings = local_string_set.strings();
  LocalSorter(local_strings, local_n);

  // There is only one PE, hence there is no need for distributed sorting 
  if (env.size() == 1) {
    return;
  }

  if constexpr (debug) {
    if (env.rank() == 0) { std::cout << "Begin sampling" << std::endl; }
    env.barrier();
  }

  auto nr_splitters = std::min<std::size_t>(env.size() - 1, local_n);
  auto splitter_dist = local_n / (nr_splitters + 1);
  std::vector<dsss::char_type> raw_splitters;

  for (std::size_t i = 1; i <= nr_splitters; ++i) {
    const auto splitter = local_strings[i * splitter_dist];
    std::copy_n(splitter, dsss::string_length(splitter) + 1,
      std::back_inserter(raw_splitters));
  }

  // Gather all splitters and sort them to determine the final splitters
  dsss::string_set splitters =
    dsss::mpi::allgather_strings(raw_splitters, env);

  if constexpr (debug) {
    if (env.rank() == 0) { std::cout << "Received all splitters" << std::endl; }
    env.barrier();
  }

  LocalSorter(splitters.strings(), splitters.size());

  nr_splitters = std::min<std::size_t>(env.size() - 1, splitters.size());
  splitter_dist = splitters.size() / (nr_splitters + 1);
  raw_splitters.clear();
  for (std::size_t i = 1; i <= nr_splitters; ++i) {
    const auto splitter = splitters[i * splitter_dist];
    std::copy_n(splitter, dsss::string_length(splitter) + 1,
      std::back_inserter(raw_splitters));
  }
  splitters = dsss::string_set(std::move(raw_splitters));

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Use " << splitters.size() << " global splitters to "
                    "determine intervals" << std::endl;
    }
    env.barrier();
  }

  // Now we need to split the local strings using the splitters
  // The size is given by the NUMBER of strings, not their lengths. The number
  // of characters that must be send to other PEs is computed in the alltoall-
  // function.
  std::vector<std::size_t> interval_sizes;
  std::size_t element_pos = 0;
  splitter_dist = local_n / (nr_splitters + 1);
  for (std::size_t i = 0; i < splitters.size(); ++i) {
    element_pos = (i + 1) * splitter_dist;
    while(element_pos > 1 && !dsss::string_smaller_eq(
      local_strings[element_pos], splitters[i])) { --element_pos; }
    while (element_pos < local_n && dsss::string_smaller_eq(
      local_strings[element_pos], splitters[i])) { ++element_pos; }
    interval_sizes.emplace_back(element_pos);
  }
  interval_sizes.emplace_back(local_n);
  for (std::size_t i = interval_sizes.size() - 1; i > 0; --i) {
    interval_sizes[i] -= interval_sizes[i - 1];
  }

  std::vector<std::size_t> receiving_sizes = dsss::mpi::alltoall(interval_sizes);
  if constexpr (print_interval_details) {
    for (std::int32_t rank = 0; rank < env.size(); ++rank) {
      if (env.rank() == rank) {
        std::size_t total_size = 0;
        std::cout << "### Sending interval sizes on PE " << rank << std::endl;
        for (const auto is : interval_sizes) {
          total_size += is;
          std::cout << is << ", ";
        }
        std::cout << "Total size: " << total_size << std::endl;
      }
      env.barrier();
    }
    for (std::int32_t rank = 0; rank < env.size(); ++rank) {
      if (env.rank() == rank) {
        std::size_t total_size = 0;
        std::cout << "### Receiving interval sizes on PE " << rank << std::endl;
        for (const auto is : receiving_sizes) {
          total_size += is;
          std::cout << is << ", ";
        }
        std::cout << "Total size: " << total_size << std::endl;
      }
      env.barrier();
    }
    if (env.rank() == 0) { std::cout << std::endl; }
  }

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Send strings to corresponding PEs" << std::endl;
    }
    env.barrier();
  }
  
  local_string_set.update(std::move(
    dsss::mpi::alltoallv_strings(local_string_set, interval_sizes)));

  std::vector<decltype(local_string_set.cbegin())> string_it(
    env.size(), local_string_set.cbegin());
  std::vector<decltype(local_string_set.cbegin())> end_it(
    env.size(), local_string_set.cbegin() + receiving_sizes[0]);

  for (std::int32_t i = 1; i < env.size(); ++i) {
    string_it[i] = string_it[i - 1] + receiving_sizes[i - 1];
    end_it[i] = end_it[i - 1] + receiving_sizes[i];
  }

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Sort received strings" << std::endl;
    }
    env.barrier();
  }

  struct string_compare {
    bool operator ()(const dsss::string a, const dsss::string b) {
      return (dsss::string_cmp(a, b) < 0);
    }
  }; // struct string_compare

  tlx::LoserTreeCopy<false, dsss::string, string_compare> lt(env.size());

  std::size_t filled_sources = 0;
  for (std::int32_t i = 0; i < env.size(); ++i) {
    if (string_it[i] == end_it[i]) { lt.insert_start(nullptr, i, true); }
    else {
      lt.insert_start(&*string_it[i], i, false);
      ++filled_sources;
    }
  }

  lt.init();

  std::vector<dsss::string> result;
  result.reserve(local_string_set.size());
  while (filled_sources) {
    std::int32_t source = lt.min_source();
    result.push_back(*string_it[source]);
    ++string_it[source];
    if (string_it[source] != end_it[source]) {
      lt.delete_min_insert(&*string_it[source], false);
    } else {
      lt.delete_min_insert(nullptr, true);
      --filled_sources;
    }
  }
  local_string_set.update(std::move(result));

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Finished" << std::endl;
    }
    env.barrier();
  }
}

// Interface of the non-templated distributed sample sort, i.e., the one that
// does not accept indexed string sets.
void sample_sort_stdsort(dsss::string_set& local_strings);

} // namespace dsss::sample_sort

/******************************************************************************/
