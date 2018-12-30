/*******************************************************************************
 * dsss/mpi/zip.hpp
 *
 * Copyright (c) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <vector>

#include "mpi/distribute_data.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"

#include "util/drop.hpp"
#include "util/function_traits.hpp"

namespace dsss::mpi {

template <typename DataType, class IndexFunction>
inline auto zip_with_index(const std::vector<DataType>& data,
  IndexFunction indexer, [[maybe_unused]] environment env = environment()) {

  size_t local_size = data.size();
  const size_t offset = ex_prefix_sum(local_size);

  std::vector<
    typename dsss::util::function_traits<IndexFunction>::result_type> result;

  result.reserve(local_size);
  for (size_t i = 0; i < local_size; ++i) {
    result.emplace_back(indexer(offset + i, data[i]));
  }

  return result;
}

template <typename FirstDataType, typename SecondDataType, typename ZipFunction>
inline auto zip(std::vector<FirstDataType>& first_data,
  std::vector<SecondDataType>& second_data, ZipFunction zipper,
  [[maybe_unused]] environment env = environment()) {

  first_data = dsss::mpi::distribute_data(first_data, env);
  second_data = dsss::mpi::distribute_data(second_data, env);
  const size_t local_size = first_data.size();

  second_data.resize(local_size);
  std::vector<
    typename dsss::util::function_traits<ZipFunction>::result_type> result;
  result.reserve(local_size);

  for (size_t i = 0; i < local_size; ++i) {
    result.emplace_back(zipper(first_data[i], second_data[i]));
  }

  return result;
}

} // namespace dsss::mpi

/******************************************************************************/
