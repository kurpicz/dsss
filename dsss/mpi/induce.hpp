/*******************************************************************************
 * mpi/induce.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 * 
 ******************************************************************************/

#pragma once

#include <vector>

#include "mpi/alltoall.hpp"
#include "mpi/allreduce.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"

#include "suffix_sorting/data_structs.hpp"

namespace dsss::mpi {

template <typename DataType, size_t max_char>
class inducer {

public:
  inducer(environment env = environment()) : env_comm_(env.communicator()),
                                             env_rank_(env.rank()),
                                             env_size_(env.size()),
                                             mpi_datatype_(dtm_.get_mpi_type()),
                                             sc_buffer_(env_size_, 0),
                                             rc_buffer_(env_size_, 0),
                                             sd_buffer_(env_size_, 0),
                                             rd_buffer_(env_size_, 0) { }

  template <bool left_to_right = true>
  inline size_t alltoallv_compute_target(DataType* send_data,
                                         DataType* cur_target_pos) {
      
    for (int32_t i = 1; i < env_size_; ++i) {
      sd_buffer_[i] = sd_buffer_[i - 1] + sc_buffer_[i - 1];
      rd_buffer_[i] = rd_buffer_[i - 1] + rc_buffer_[i - 1];
    }

    size_t recv_elements = rc_buffer_.back() + rd_buffer_.back();
 
    if constexpr (left_to_right) {
        MPI_Alltoallv(send_data,
                      sc_buffer_.data(),
                      sd_buffer_.data(),
                      mpi_datatype_,
                      cur_target_pos,
                      rc_buffer_.data(),
                      rd_buffer_.data(),
                      mpi_datatype_,
                      env_comm_);
    } else /*if (!left_to_right)*/ {
      MPI_Alltoallv(send_data,
                    sc_buffer_.data(),
                    sd_buffer_.data(),
                    mpi_datatype_,
                    cur_target_pos - recv_elements,
                    rc_buffer_.data(),
                    rd_buffer_.data(),
                    mpi_datatype_,
                    env_comm_);
    }
    return recv_elements;
  }

  void induce_right_to_left(std::vector<DataType*>& send_data,
                            std::vector<size_t>& local_counts,
                            std::vector<size_t>& local_containing,
                            std::vector<size_t>& global_bucket_sizes,
                            std::vector<DataType*>& cur_target_pos,
                            std::vector<dsss::suffix_sorting::bucket_info<DataType>*>& tar_buckets) {

    size_t const considered_chars = local_counts.size();

    std::vector<size_t> preceding_sizes(considered_chars);
    MPI_Allreduce(local_containing.data(),
                  preceding_sizes.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  MPI_SUM,
                  env_comm_);
    
    std::vector<size_t> prec_send_buffer(env_size_ * considered_chars, 0);
    MPI_Allgather(local_counts.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  prec_send_buffer.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  env_comm_);

    for (size_t i = 0; i < considered_chars; ++i) {
      for (int32_t j = env_size_ - 1; j > env_rank_; --j) {
        preceding_sizes[i] += prec_send_buffer[i + (j * considered_chars)];
      } 
    }

    auto target_rank = [=](size_t const pos, size_t const sz) -> int32_t {
      if (sz == 0) {
        return 0;
      }
      return env_size_ - 1 - std::min<int32_t>(pos / sz, env_size_ - 1);
    };

    std::vector<int32_t> send_count_buffer(env_size_ * considered_chars, 0);
    for (size_t i = 0; i < considered_chars; ++i) {
      size_t missing_slice = 0;
      size_t const slice_size = global_bucket_sizes[i] / env_size_;
      for (int32_t cur_rank = target_rank(preceding_sizes[i], slice_size);
           local_counts[i] > 0 && cur_rank >= 0; --cur_rank) {
        if (cur_rank == 0) {
          missing_slice = global_bucket_sizes[i] % env_size_;
        }
        const size_t to_send = std::min((env_size_ - cur_rank) * slice_size +
                                        missing_slice - preceding_sizes[i],
                                        local_counts[i]);
        send_count_buffer[i + (considered_chars * cur_rank)] = to_send;
        local_counts[i] -= to_send;
        preceding_sizes[i] += to_send;
      }
    }

    std::vector<int32_t> receive_count_buffer(env_size_ * considered_chars, 0);
    MPI_Alltoall(send_count_buffer.data(),
                 considered_chars,
                 MPI_INT,
                 receive_count_buffer.data(),
                 considered_chars,
                 MPI_INT,
                 env_comm_);

    // environment env;
    // for (int32_t i = 0; i < env.size(); ++i) {
    //   if (i == env.rank()) {
    //     std::cout << "i " << i << ": RECEIVE COUNTS" << std::endl;
    //     size_t j = 0;
    //     for (const auto& cnt : receive_count_buffer) {
    //       std::cout << "cnt[" << j++ << "] " << cnt << ", ";
    //     }
    //     std::cout << std::endl;
    //   }
    //   env.barrier();
    // }


    for (size_t i = 0; i < considered_chars; ++i) {
      for (int32_t j = 0; j < env_size_; ++j) {
        sc_buffer_[j] = send_count_buffer[i + (j * considered_chars)];
        rc_buffer_[j] = receive_count_buffer[i + (j * considered_chars)];
      }
      size_t const res = alltoallv_compute_target<false>(send_data[i],
                                                         cur_target_pos[i]);
      tar_buckets[i]->containing += res;
    }
  }

  void induce_left_to_right(std::vector<DataType*>& send_data,
                            std::vector<size_t>& local_counts,
                            std::vector<size_t>& local_containing,
                            std::vector<size_t>& global_bucket_sizes,
                            std::vector<DataType*>& cur_target_pos,
                            std::vector<dsss::suffix_sorting::bucket_info<DataType>*>& tar_buckets) {

    size_t const considered_chars = local_counts.size();

    std::vector<size_t> preceding_sizes(considered_chars);
    MPI_Allreduce(local_containing.data(),
                  preceding_sizes.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  MPI_SUM,
                  env_comm_);
    
    std::vector<size_t> prec_send_buffer(env_size_ * considered_chars, 0);
    MPI_Allgather(local_counts.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  prec_send_buffer.data(),
                  considered_chars,
                  MPI_UNSIGNED_LONG_LONG,
                  env_comm_);

    for (size_t i = 0; i < considered_chars; ++i) {
      for (int32_t j = 0; j < env_rank_; ++j) {
        preceding_sizes[i] += prec_send_buffer[i + (j * considered_chars)];
      } 
    }

    auto target_rank = [=](size_t const pos, size_t const sz) -> int32_t {
      if (sz == 0) {
        return env_size_ - 1;
      }
      return std::min<int32_t>(pos / sz, env_size_ - 1);
    };

    std::vector<int32_t> send_count_buffer(env_size_ * considered_chars, 0);
    for (size_t i = 0; i < considered_chars; ++i) {
      size_t missing_slice = 0;
      size_t const slice_size = global_bucket_sizes[i] / env_size_;
      for (int32_t cur_rank =  target_rank(preceding_sizes[i], slice_size);
           local_counts[i] > 0 && cur_rank >= 0; ++cur_rank) {
        if (cur_rank + 1 == env_size_) {
          missing_slice = global_bucket_sizes[i] % env_size_;
        }
        const size_t to_send = std::min((cur_rank + 1) * slice_size +
                                        missing_slice - preceding_sizes[i],
                                        local_counts[i]);
        send_count_buffer[i + (considered_chars * cur_rank)] = to_send;
        local_counts[i] -= to_send;
        preceding_sizes[i] += to_send;
      }
    }
    std::vector<int32_t> receive_count_buffer(env_size_ * considered_chars, 0);
    MPI_Alltoall(send_count_buffer.data(),
                 considered_chars,
                 MPI_INT,
                 receive_count_buffer.data(),
                 considered_chars,
                 MPI_INT,
                 env_comm_);

    for (size_t i = 0; i < considered_chars; ++i) {
      for (int32_t j = 0; j < env_size_; ++j) {
        sc_buffer_[j] = send_count_buffer[i + (j * considered_chars)];
        rc_buffer_[j] = receive_count_buffer[i + (j * considered_chars)];
      }
      size_t const res = alltoallv_compute_target<true>(send_data[i],
                                                        cur_target_pos[i]);
      tar_buckets[i]->containing += res;
    }
  }


private:
  MPI_Comm const env_comm_;
  int32_t const env_rank_;
  int32_t const env_size_;
  data_type_mapper<DataType> dtm_;
  MPI_Datatype const mpi_datatype_;
  std::vector<int32_t> sc_buffer_;
  std::vector<int32_t> rc_buffer_;
  std::vector<int32_t> sd_buffer_;
  std::vector<int32_t> rd_buffer_;

  static constexpr size_t width_ = max_char + 1;
}; // class induce

} // namespace dsss::mpi

/******************************************************************************/
