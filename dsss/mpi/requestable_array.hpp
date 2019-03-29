/*******************************************************************************
 * suffix_sorting/requestable_array.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>
#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"
#include "mpi/type_mapper.hpp"

namespace dsss::mpi {

template <typename DataType>
class requestable_array {

public:
  requestable_array(std::vector<DataType>& data,
                    size_t total_size,
                    environment env = environment())
    : local_size_(data.size()), slice_size_(total_size / env.size()),
      data_(data.data()), env_(env) {
    
    MPI_Win_create(data_,
                   local_size_,
                   sizeof(DataType),
                   MPI_INFO_NULL,
                   env_.communicator(),
                   &win_);
  }

  requestable_array(size_t const local_size, DataType* data,
                    environment env = environment())
    : local_size_(local_size), data_(data), env_(env){

    MPI_Win_create(data_,
                   local_size_,
                   sizeof(DataType),
                   MPI_INFO_NULL,
                   env_.communicator(),
                   &win_);
  }

  inline DataType& operator [](size_t index) {
    return data_[index];
  }

  inline DataType operator [](size_t index) const {
    return data_[index];
  }

  inline DataType& back() {
    return data_[local_size_ - 1];
  }

  inline DataType back() const {
    return data_[local_size_ - 1];
  }

  template <typename IndexType>
  std::vector<DataType> request2(std::vector<IndexType>& request_positions) {

    auto compute_target_rank = [&](IndexType pos) {
      return std::min<int32_t>(pos / slice_size_, env_.size() - 1);
    };

    std::vector<int32_t> hist(env_.size(), 0);
    std::vector<int32_t> ranks(request_positions.size());
    for (size_t i = 0; i < request_positions.size(); ++i) {
      const int32_t rank = compute_target_rank(request_positions[i]);
      ++hist[rank];
      ranks[i] = rank;
    }

    std::vector<int32_t> starting_positions(env_.size(), 0);
    for (int32_t i = 1; i < env_.size(); ++i) {
      starting_positions[i] = starting_positions[i - 1] + hist[i - 1];
    }

    std::vector<int32_t> normalize_pos(request_positions.size(), 0);
    for (size_t i = 0; i < request_positions.size(); ++i) {
      normalize_pos[starting_positions[ranks[i]]++] =
        request_positions[i] - (ranks[i] * slice_size_);
    }

    auto [rec_count, rec_req] = alltoallv_counts(normalize_pos, hist);
    for (auto& req : rec_req) {
      req = data_[req];
    }

    normalize_pos = alltoallv_small(rec_req, rec_count);
    starting_positions[0] = 0;
    for (int32_t i = 1; i < env_.size(); ++i) {
      starting_positions[i] = starting_positions[i - 1] + hist[i - 1];
    }

    std::vector<DataType> result(request_positions.size());
    for (size_t i = 0; i < request_positions.size(); ++i) {
      result[i] = normalize_pos[starting_positions[ranks[i]]++];
    }
    return result;
  }

private:
  size_t offset_;
  size_t local_size_;
  size_t slice_size_;
  DataType* data_;
  environment env_;
  data_type_mapper<DataType> dtm_;

  static constexpr size_t req_round_size = 1024 * 1024;

  MPI_Win win_;

}; // class requestable_array

} // namespace dsss::mpi

/******************************************************************************/
