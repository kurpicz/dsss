/*******************************************************************************
 * mpi/sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <vector>

#include <JanusSort.hpp>
#include <RQuick.hpp>
#include "ips4o.hpp"
#include <tlx/container/loser_tree.hpp>

#include "mpi/allgather.hpp"
#include "mpi/allreduce.hpp"
#include "mpi/distribute_data.hpp"
#include "mpi/environment.hpp"

#include "util/macros.hpp"

namespace dsss::mpi {

template <typename DataType, class Compare>
inline void sort(std::vector<DataType>& local_data, MPI_Datatype mpi_data_type,
                 Compare comp, environment env = environment()) {

  local_data = dsss::mpi::distribute_data(local_data, env);

  // Sort locally
  ips4o::sort(local_data.begin(), local_data.end(), comp);

  // Compute the local splitters given the sorted data
  const size_t local_n = local_data.size();
  auto nr_splitters = static_cast<size_t>(std::min<size_t>(env.size() * 20 - 1,
                                                           local_n));
  auto splitter_dist = local_n / (nr_splitters + 1);

  std::vector<DataType> local_splitters;
  local_splitters.reserve(nr_splitters);
  for (size_t i = 1; i <= nr_splitters; ++i) {
    local_splitters.emplace_back(local_data[i * splitter_dist]);
  }

  JanusSort::sort(env.communicator(), local_splitters, mpi_data_type, comp);

  /* RQuick */
  // int tag = 11111;
  // std::mt19937_64 generator;
  // int data_seed = 3469931 + env.rank();
  // generator.seed(data_seed);
  // // RBC::Comm rcomm;
  // // RBC::Create_Comm_from_MPI(env.communicator(), &rcomm);
  // auto comm = env.communicator();

  // RQuick::sort(generator, local_splitters, mpi_data_type, tag, comm, comp);

  DataType smpl_splitter = local_splitters.back();

  local_splitters = dsss::mpi::allgather(smpl_splitter, env);
  local_splitters.pop_back();

  // Use the final set of splitters to find the intervals
  std::vector<size_t> interval_sizes;
  size_t element_pos = 0;
  splitter_dist = local_n / (nr_splitters + 1);

  for (size_t i = 0; i < local_splitters.size(); ++i) {
    element_pos = ((i + 1) * splitter_dist);
    while(element_pos > 0 && !comp(
      local_data[element_pos], local_splitters[i])) { --element_pos; }
    while (element_pos < local_n && comp(
      local_data[element_pos], local_splitters[i])) { ++element_pos; }
    interval_sizes.emplace_back(element_pos);
  }
  interval_sizes.emplace_back(local_n);
  for (size_t i = interval_sizes.size() - 1; i > 0; --i) {
    interval_sizes[i] -= interval_sizes[i - 1];
  }

  std::vector<size_t> receiving_sizes = alltoall(interval_sizes);

  for (int64_t i = interval_sizes.size(); i < env.size(); ++i) {
    interval_sizes.emplace_back(0);
  }

  local_data = alltoallv(local_data, interval_sizes, env);

  if (local_data.size() == 0) {
    return;
  }

  if (false && local_data.size() < 1024 * 1024) {
    std::vector<decltype(local_data.cbegin())> string_it(
      env.size(), local_data.cbegin());
    std::vector<decltype(local_data.cbegin())> end_it(
      env.size(), local_data.cbegin() + receiving_sizes[0]);

    size_t received_elements = receiving_sizes[0];
    for (int32_t i = 1; i < env.size(); ++i) {
      string_it[i] = string_it[i - 1] + receiving_sizes[i - 1];
      received_elements += receiving_sizes[i];
      end_it[i] = end_it[i - 1] + receiving_sizes[i];
    }

    struct item_compare {
      item_compare(Compare compare) : comp_(compare) { }

      bool operator ()(const DataType& a, const DataType& b) {
        return comp_(a, b);
      }

    private:
      Compare comp_;
    }; // struct item_compare

    tlx::LoserTreeCopy<false, DataType, item_compare> lt(env.size(),
      item_compare(comp));

    size_t filled_sources = 0;
    for (int32_t i = 0; i < env.size(); ++i) {
      if (string_it[i] >= end_it[i]) { lt.insert_start(nullptr, i, true); }
      else {
        lt.insert_start(&*string_it[i], i, false);
        ++filled_sources;
      }
    }

    lt.init();

    std::vector<DataType> result;
    result.reserve(local_data.size());
    while (filled_sources) {
      int32_t source = lt.min_source();
      result.push_back(*string_it[source]);
      ++string_it[source];
      if (string_it[source] < end_it[source]) {
        lt.delete_min_insert(&*string_it[source], false);
      } else {
        lt.delete_min_insert(nullptr, true);
        --filled_sources;
      }
    }
    local_data = std::move(result);
  } else if (local_data.size() > 0) {
    ips4o::sort(local_data.begin(), local_data.end(), comp);
  }
}

} // namespace dsss::mpi

/******************************************************************************/
