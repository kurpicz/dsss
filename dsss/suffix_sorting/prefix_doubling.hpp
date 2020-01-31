/*******************************************************************************
 * suffix_sorting/prefix_doubling.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <vector>

#include <tlx/math.hpp>

#include "mpi/allreduce.hpp"
#include "mpi/environment.hpp"
#include "mpi/scan.hpp"
#include "mpi/shift.hpp"
#include "mpi/sort.hpp"
#include "mpi/gather.hpp"
#include "suffix_sorting/data_structs.hpp"
#include "util/string.hpp"

namespace dsss::suffix_sorting {

  static constexpr bool debug = false;
  static constexpr bool debug_time_only = false;

template <typename IndexType>
inline auto pack_alphabet(dsss::distributed_string& distributed_raw_string,
  size_t& iteration) {

  using IRR = index_rank_rank<IndexType>;

  dsss::mpi::environment env;

  std::vector<dsss::char_type>& local_str = distributed_raw_string.string;
  std::vector<size_t> char_histogram(256, 0);
  for (const auto c : local_str) { ++char_histogram[c]; }
  char_histogram = dsss::mpi::allreduce_sum(char_histogram, env);
  std::vector<size_t> char_map(256, 0);
  size_t new_alphabet_size = 1;
  for (size_t i = 0; i < 256; ++i) {
    if (char_histogram[i] != 0) { char_map[i] = new_alphabet_size++; }
  }
  size_t bits_per_char = tlx::integer_log2_ceil(new_alphabet_size);
  size_t k_fitting = (8 * sizeof(IndexType)) / bits_per_char;
  iteration = tlx::integer_log2_floor(k_fitting) + 1;

  if constexpr (debug) {
    std::vector<size_t> global_histogram = dsss::mpi::allreduce_sum(
      char_histogram, env);

    if (env.rank() == 0) {
      std::cout << "Text-Histogram:" << std::endl;
      for (size_t i = 0; i < 256; ++i) {
        if (size_t occ = char_histogram[i]; occ > 0) {
          std::cout << i << ": " << occ << " ("
                    << (100 / double(local_str.size())) * occ << "%)"
                    << std::endl;
        }
      }
      std::cout << "New alphabet size = " << new_alphabet_size << std::endl
                << "Requiring " << bits_per_char << " bits per character."
                                                 << std::endl 
                << "Packing " << k_fitting << " characters in one rank."
                                           << std::endl
                << "Starting at iteration " << iteration << "." << std::endl;
    }
  }

  size_t local_size = local_str.size();
  std::vector<dsss::char_type> right_chars = dsss::mpi::shift_left(
    local_str.data(), 2 * k_fitting, env);
  if (env.rank() + 1 < env.size()) {
    std::move(right_chars.begin(), right_chars.end(),
      std::back_inserter(local_str));
  } else {
    for (size_t i = 0; i < 2 * k_fitting; ++i) {
      local_str.emplace_back(0);
    }
  }

  size_t index = dsss::mpi::ex_prefix_sum(local_size, env);
  std::vector<IRR> result;
  result.reserve(local_size);
  for (size_t i = 0; i < local_size; ++i) {
    IndexType rank1 = IndexType(char_map[local_str[i]]);
    IndexType rank2 = IndexType(char_map[local_str[i + k_fitting]]);
    for (size_t j = 1; j < k_fitting; ++j) {
      rank1 = (rank1 << bits_per_char) | char_map[local_str[i + j]];
      rank2 = (rank2 << bits_per_char) | char_map[local_str[i + k_fitting + j]];
    }
    result.emplace_back(index++, rank1, rank2);
  }
  return result;
}

template <typename IndexType>
void prefix_doubling_local(
  std::vector<index_rank_state<IndexType>>& local_irss,
  size_t iteration,
  dsss::mpi::environment env = dsss::mpi::environment()) {

  using IRR = index_rank_rank<IndexType>;
  using IRS = index_rank_state<IndexType>;

  if constexpr (debug) {
    if (env.rank() == 0) {
      std::cout << "Sorting locally" << std::endl;
    }
  }

  int32_t const root = 0;

  auto irss = dsss::mpi::gatherv(local_irss, 0, env);

  if (env.rank() == root) {
    size_t const local_size = irss.size();
    std::vector<IRR> irrs(local_size);
    while (true) {
      if constexpr (debug) {
        std::cout << "Start local iteration" << std::endl;
      }
      bool all_unique = true;
      for (size_t i = 1; all_unique && i < local_size; ++i) {
        all_unique = irss[i].rank != irss[i - 1].rank;
      }
      if (all_unique) {
        break;
      }
      
      if constexpr (debug) {
        std::cout << "Start local mod/div sort" << std::endl;
      }
      std::sort(irss.begin(), irss.end(),
                [iteration](const IRS& a, const IRS& b) {
                  IndexType const mod_mask = (size_t(1) << iteration) - 1;
                  IndexType const div_mask = ~mod_mask;

                  if ((a.index & mod_mask) == (b.index & mod_mask)) {
                    return (a.index & div_mask) < (b.index & div_mask);
                  } else {
                    return (a.index & mod_mask) < (b.index & mod_mask);
                  }
      });

      if constexpr (debug) {
        std::cout << "Compute new local ranks" << std::endl;
      }

      irss.emplace_back(0 , 0, rank_state::UNIQUE);
      const size_t index_distance = size_t(1) << iteration;
      for (size_t i = 0; i < local_size; ++i) {
        IndexType second_rank = { 0 };
        if (DSSS_LIKELY(irss[i].index + index_distance == irss[i + 1].index)) {
          second_rank = irss[i + 1].rank;
        }
        irrs[i] = IRR { irss[i].index,irss[i].rank, second_rank };
      }
      irss.pop_back();

      if constexpr (debug) {
        std::cout << "Sort by rank tuples locally" << std::endl;
      }

      std::sort(irrs.begin(), irrs.end(), [](IRR const& a, IRR const& b) {
        return std::tie(a.rank1, a.rank2) < std::tie(b.rank1, b.rank2);
      });

      IndexType cur_rank = irrs[0].rank1;
      IndexType same_rank = 1;
      irss[0] = IRS { irrs[0].index, irrs[0].rank1, rank_state::UNIQUE };
      for (size_t i = 1; i < local_size; ++i) {
        if (irrs[i].rank1 == irrs[i - 1].rank1) {
          if (irrs[i].rank2 != irrs[i - 1].rank2 ||
              irrs[i].rank2 == IndexType { 0 }) {
            cur_rank = irrs[i].rank1 + same_rank;
          }
          ++same_rank;
        } else {
          same_rank = 1;
          cur_rank = irrs[i].rank1;
        }

        irss[i] = IRS { irrs[i].index, cur_rank, rank_state::UNIQUE };
      }
      if constexpr (debug) {
        std::cout << "Finished local iteration " << iteration << std::endl;
      }
      ++iteration;
    }
  }

  local_irss = dsss::mpi::distribute_data(irss, env);
}

template <typename IndexType>
std::vector<IndexType> prefix_doubling(dsss::distributed_string&&
  distributed_raw_string) {

  using IR = index_rank<IndexType>;
  using IRR = index_rank_rank<IndexType>;

  MPI_Datatype mpi_index_type;
  MPI_Type_contiguous(sizeof(IndexType), MPI_CHAR, &mpi_index_type);
  MPI_Type_commit(&mpi_index_type);

  constexpr size_t ir_num_members = 2;
  int32_t ir_lengths[ir_num_members] = { 1, 1 };
  MPI_Aint ir_offsets[ir_num_members] = { offsetof(IR, index),
                                          offsetof(IR, rank) };
  MPI_Datatype ir_types[ir_num_members] = { mpi_index_type, mpi_index_type };

  MPI_Datatype mpi_ir_type;
  MPI_Type_create_struct(ir_num_members, ir_lengths, ir_offsets, ir_types,
                         &mpi_ir_type);
  MPI_Type_commit(&mpi_ir_type);

  constexpr size_t irr_num_members = 3;
  int32_t irr_lengths[irr_num_members] = { 1, 1, 1 };
  MPI_Aint irr_offsets[irr_num_members] = { offsetof(IRR, index),
                                            offsetof(IRR, rank1),
                                            offsetof(IRR, rank2) };
  MPI_Datatype irr_types[irr_num_members] = { mpi_index_type, mpi_index_type,
                                              mpi_index_type };

  MPI_Datatype mpi_irr_type;
  MPI_Type_create_struct(irr_num_members, irr_lengths, irr_offsets, irr_types,
                         &mpi_irr_type);
  MPI_Type_commit(&mpi_irr_type);

  dsss::mpi::environment env;

  size_t offset = 0;
  size_t local_size = 0;
  size_t iteration = 0;
  std::vector<IRR> irrs = pack_alphabet<IndexType>(distributed_raw_string,
                                                   iteration);
  std::vector<IR> irs;
  while (true) {
    auto start_time = MPI_Wtime();

    // Sort based on two ranks
    dsss::mpi::sort(irrs, mpi_irr_type, [](const IRR& a, const IRR& b) {
      return std::tie(a.rank1, a.rank2) < std::tie(b.rank1, b.rank2); },
      env);
    
    // Compute new ranks
    local_size = irrs.size();
    offset = dsss::mpi::ex_prefix_sum(local_size);

    irs.clear();
    irs.reserve(local_size);

    size_t cur_rank = offset;
    irs.emplace_back(irrs[0].index, cur_rank);
    for (size_t i = 1; i < local_size; ++i) {
      if (irrs[i - 1] != irrs[i]) { cur_rank = offset + i; }
      irs.emplace_back(irrs[i].index, cur_rank);
    }

    bool all_distinct = true;
    for (size_t i = 1; all_distinct && i < irs.size(); ++i) {
      all_distinct &= (irs[i].rank != irs[i - 1].rank);
      if (!all_distinct) { break; }
    }
    const bool finished = dsss::mpi::allreduce_and(all_distinct, env);
    if (finished) { break; }

    dsss::mpi::sort(irs, mpi_ir_type, [iteration](const IR& a, const IR& b) {
        IndexType const mod_mask = (size_t(1) << iteration) - 1;
        IndexType const div_mask = ~mod_mask;

        if ((a.index & mod_mask) == (b.index & mod_mask)) {
          return (a.index & div_mask) < (b.index & div_mask);
        } else {
          return (a.index & mod_mask) < (b.index & mod_mask);
        }
    }, env);

    local_size = irs.size();
    offset = dsss::mpi::ex_prefix_sum(local_size);

    IR rightmost_ir = dsss::mpi::shift_left(irs.front(), env);
    if (env.rank() + 1 < env.size()) { irs.emplace_back(rightmost_ir); }
    else { irs.emplace_back(0, 0); }

    irrs.clear();
    irrs.reserve(local_size);
    const size_t index_distance = size_t(1) << iteration;
    for (size_t i = 0; i < local_size; ++i) {
      IndexType second_rank = { 0 };
      if (DSSS_LIKELY(irs[i].index + index_distance == irs[i + 1].index)) {
        second_rank = irs[i + 1].rank;
      }
      irrs.emplace_back(irs[i].index, irs[i].rank, second_rank);
    }

    env.barrier();
    auto end_time = MPI_Wtime();

    if constexpr (debug_time_only || debug) {
      if (env.rank() == 0) {
        std::cout << "Finished iteration " << iteration << " (w/o discarding)"
                  << " in " << end_time - start_time << " seconds"
                  << std::endl;
      }
    }


    ++iteration;
  }

  std::vector<IndexType> result;
  result.reserve(irs.size());
  std::transform(irs.begin(), irs.end(), std::back_inserter(result),
    [](const IR& ir) { return ir.index; });
  return result;
}

template <typename IndexType, bool return_isa = false>
std::vector<IndexType> doubling_discarding(
  std::vector<index_rank_state<IndexType>>& irss,
  size_t iteration,
  dsss::mpi::environment env = dsss::mpi::environment()) {

  using IR = index_rank<IndexType>;
  using IRS = index_rank_state<IndexType>;
  using IRR = index_rank_rank<IndexType>;

  MPI_Datatype mpi_index_type;
  MPI_Type_contiguous(sizeof(IndexType), MPI_CHAR, &mpi_index_type);
  MPI_Type_commit(&mpi_index_type);

  constexpr size_t ir_num_members = 2;
  int32_t ir_lengths[ir_num_members] = { 1, 1 };
  MPI_Aint ir_offsets[ir_num_members] = { offsetof(IR, index),
                                          offsetof(IR, rank) };
  MPI_Datatype ir_types[ir_num_members] = { mpi_index_type, mpi_index_type };

  MPI_Datatype mpi_ir_type;
  MPI_Type_create_struct(ir_num_members, ir_lengths, ir_offsets, ir_types,
                         &mpi_ir_type);
  MPI_Type_commit(&mpi_ir_type);

  constexpr size_t irr_num_members = 3;
  int32_t irr_lengths[irr_num_members] = { 1, 1, 1 };
  MPI_Aint irr_offsets[irr_num_members] = { offsetof(IRR, index),
                                            offsetof(IRR, rank1),
                                            offsetof(IRR, rank2) };
  MPI_Datatype irr_types[irr_num_members] = { mpi_index_type, mpi_index_type,
                                              mpi_index_type };

  MPI_Datatype mpi_irr_type;
  MPI_Type_create_struct(irr_num_members, irr_lengths, irr_offsets, irr_types,
                         &mpi_irr_type);
  MPI_Type_commit(&mpi_irr_type);

  constexpr size_t irs_num_members = 3;
  int32_t irs_lengths[irs_num_members] = { 1, 1, 1};
  MPI_Aint irs_offsets[irs_num_members] = { offsetof(IRS, index),
                                            offsetof(IRS, rank),
                                            offsetof(IRS, state) };
  MPI_Datatype irs_types[irs_num_members] = { mpi_index_type, mpi_index_type,
                                              MPI_UINT8_T };
  MPI_Datatype mpi_irs_type;
  MPI_Type_create_struct(irs_num_members, irs_lengths, irs_offsets, irs_types,
                         &mpi_irs_type);
  MPI_Type_commit(&mpi_irs_type);

  std::vector<IRR> irrs;
  std::vector<IR> fully_discarded;

  while (iteration) {
    double start_time;

    if constexpr (debug_time_only || debug) {
      start_time = MPI_Wtime();
    }

    if constexpr (debug) {
      env.barrier();
      if (env.rank() == 0) {
        std::cout << "Start of iteration " << iteration << std::endl;
      }
      env.barrier();
      size_t local_irss = irss.size();

      size_t global_irss = dsss::mpi::allreduce_sum(local_irss);

      for (int32_t i = 0; i < env.size(); ++i) {
        if (i == env.rank()) {
          std::cout << i << ": "
                    << "local_elements " << local_irss
                    << " (" << 100.0 / global_irss * local_irss << "%)"
                    << std::endl;
        }
        env.barrier();
      }
    }

    size_t local_undecided = irss.size();
    size_t global_undecided = dsss::mpi::allreduce_sum(local_undecided);

    // if (global_undecided < static_cast<size_t>(1024 * env.size())) {
    //   prefix_doubling_local(irss, iteration, env);
    //   break;
    // }

    dsss::mpi::sort(irss, mpi_irs_type,
                    [iteration](const IRS& a, const IRS& b) {
                      IndexType mod_mask = (size_t(1) << iteration) - 1;
                      IndexType div_mask = ~mod_mask;

                      if ((a.index & mod_mask) == (b.index & mod_mask)) {
                        return (a.index & div_mask) < (b.index & div_mask);
                      } else {
                        return (a.index & mod_mask) < (b.index & mod_mask);
                      }
                    }, env);

    if constexpr (debug) {
      env.barrier();
      if (env.rank() == 0) {
        std::cout << "Sorting Mod/Div in iteration " << iteration << std::endl;
      }
      env.barrier();

      size_t local_irss = irss.size();
      size_t global_irss = dsss::mpi::allreduce_sum(local_irss);

      if (env.rank() == 0) {
        std::cout << "global_irss " << global_irss << std::endl;
      }
      env.barrier();

      for (int32_t i = 0; i < env.size(); ++i) {
        if (i == env.rank()) {
          std::cout << i << ": "
                    << "local_elements " << local_irss
                    << " (" << 100.0 / global_irss * local_irss << "%)"
                    << std::endl;
        }
        env.barrier();
      }
    }

    size_t local_size = irss.size();
    // IRS rightmost_irs = dsss::mpi::shift_left(irss.front(), env);
    struct optional_irs {
      bool is_empty;
      IRS  irs;
    } DSSS_ATTRIBUTE_PACKED;

    optional_irs o_irs = { local_size == 0,
      (local_size == 0) ? IRS { 0, 0, rank_state::UNIQUE } : irss.front() };
    std::vector<optional_irs> rec_data = dsss::mpi::allgather(o_irs, env);
    IRS rightmost_irs = { 0, 0, rank_state::UNIQUE };
    {
      int32_t rank = env.rank() + 1;
      while (rank < env.size() && rec_data[rank].is_empty) {
        ++rank;
      }
      if (rank < env.size()) {
        rightmost_irs = rec_data[rank].irs;
      }
    }

    if (env.rank() + 1 < env.size()) {
      irss.emplace_back(rightmost_irs);
    } else {
      irss.emplace_back(0, 0, rank_state::UNIQUE);
    }

    std::vector<IRS> unique;
    size_t prev_non_unique = 2;
    const size_t index_distance = size_t(1) << iteration;
    std::vector<IR> new_discarded;
    for (size_t i = 0; i < local_size; ++i) {
      if (DSSS_UNLIKELY(irss[i].state == rank_state::UNIQUE)) {
        if (prev_non_unique > 1) {
          irrs.emplace_back(irss[i].index, irss[i].rank, IndexType(0));
        }
        else {
          new_discarded.emplace_back(irss[i].index, irss[i].rank);
        }
        prev_non_unique = 0;
      } else {
        irrs.emplace_back(irss[i].index, irss[i].rank,
          DSSS_LIKELY(irss[i].index + index_distance == irss[i + 1].index) ?
            irss[i + 1].rank : IndexType { 0 });
        ++prev_non_unique;
      }
    }
    // unique = dsss::mpi::distribute_data(unique, env);
    new_discarded = dsss::mpi::distribute_data(new_discarded, env);
    fully_discarded.reserve(fully_discarded.size() + new_discarded.size());
    std::move(new_discarded.begin(), new_discarded.end(),
              std::back_inserter(fully_discarded));

    if constexpr (debug) {
      env.barrier();
      if (env.rank() == 0) {
        std::cout << "Discarded in iteration " << iteration << std::endl;
      }
      env.barrier();
      size_t local_discarded = fully_discarded.size();
      size_t local_unique = unique.size();
      size_t local_undecided = irrs.size();

      size_t global_discarded = dsss::mpi::allreduce_sum(local_discarded);
      size_t global_unique = dsss::mpi::allreduce_sum(local_unique);
      size_t global_undecided = dsss::mpi::allreduce_sum(local_undecided);

      for (int32_t i = 0; i < env.size(); ++i) {
        if (i == env.rank()) {
          std::cout << i << ": "
                    << "local_discarded " << local_discarded
                    << " (" << 100.0 / global_discarded * local_discarded << "%)"
                    << ", local_unique " << local_unique
                    << " (" << 100.0 / global_unique * local_unique << "%)"
                    << ", local_undecided " << local_undecided
                    << " (" << 100.0 / global_undecided * local_undecided << "%)"
                    << std::endl;
        }
        env.barrier();
      }

      if (env.rank() == 0) {
        std::cout << "Global discarded " << global_discarded << std::endl
                  << "Global unique " << global_unique << std::endl
                  << "Global undecided " << global_undecided << std::endl
                  << "Total " << global_discarded + global_unique + 
                                 global_undecided
                  << std::endl;
      }
      env.barrier();
    }

    // irss.clear();
    // irss.shrink_to_fit();

    std::remove_reference_t<decltype(irss)>(std::move(irss));

    dsss::mpi::sort(irrs, mpi_irr_type, [](const IRR& a, const IRR& b){
      return std::tie(a.rank1, a.rank2) < std::tie(b.rank1, b.rank2);
    }, env);

    if constexpr (debug) {
      env.barrier();
      if (env.rank() == 0) {
        std::cout << "Sorting rank tuples in iteration " << iteration
                  << std::endl;
      }
      env.barrier();
      size_t local_discarded = fully_discarded.size();
      size_t local_unique = unique.size();
      size_t local_undecided = irrs.size();

      size_t global_discarded = dsss::mpi::allreduce_sum(local_discarded);
      size_t global_unique = dsss::mpi::allreduce_sum(local_unique);
      size_t global_undecided = dsss::mpi::allreduce_sum(local_undecided);

      for (int32_t i = 0; i < env.size(); ++i) {
        if (i == env.rank()) {
          std::cout << i << ": "
                    << "local_discarded " << local_discarded
                    << " (" << 100.0 / global_discarded * local_discarded << "%)"
                    << ", local_unique " << local_unique
                    << " (" << 100.0 / global_unique * local_unique << "%)"
                    << ", local_undecided " << local_undecided
                    << " (" << 100.0 / global_undecided * local_undecided << "%)"
                    << std::endl;
        }
        env.barrier();
      }

      if (env.rank() == 0) {
        std::cout << "Global discarded " << global_discarded << std::endl
                  << "Global unique " << global_unique << std::endl
                  << "Global undecided " << global_undecided << std::endl
                  << "Total " << global_discarded + global_unique + 
                                 global_undecided
                  << std::endl;
      }
      env.barrier();
    }

    local_size = irrs.size();
    irss.reserve(local_size);

    struct prev_occur {
      IndexType last_first_rank;
      size_t count;
    };

    prev_occur pocc = { local_size > 0 ? irrs.back().rank1 : IndexType(0), 1 };

    while (pocc.count < local_size &&
           irrs[local_size - pocc.count - 1].rank1 == pocc.last_first_rank) {
      ++pocc.count;
    }
    if (pocc.last_first_rank == IndexType(0)) {
      pocc.count = 0;
    }

    std::vector<prev_occur> poccs = dsss::mpi::allgather(pocc, env);

    size_t offset = 0;
    size_t cur_rank = 0;
    if (local_size > 0) {
      for (int32_t i = env.rank() - 1; i >= 0 &&
           (poccs[i].last_first_rank == IndexType(0) ||
            irrs[0].rank1 == poccs[i].last_first_rank); --i) {
        offset += poccs[i].count;
      }
      cur_rank = irrs[0].rank1 + offset;
      ++offset;
      irss.emplace_back(irrs[0].index, cur_rank, rank_state::UNIQUE);
    }
    for (size_t i = 1; i < local_size; ++i) {
      if (irrs[i].rank1 == irrs[i - 1].rank1) {
        if (irrs[i].rank2 != irrs[i - 1].rank2) {
          cur_rank = irrs[i].rank1 + offset;
        }
        ++offset;
      } else {
        offset = 1;
        cur_rank = irrs[i].rank1;
      }
      irss.emplace_back(irrs[i].index, cur_rank, rank_state::UNIQUE);
    }

    if constexpr (debug) {
      env.barrier();
      if (env.rank() == 0) {
        std::cout << "Generating new ranks in iteration " << iteration
                  << std::endl;
      }
      env.barrier();
    }

    std::remove_reference_t<decltype(irrs)>(std::move(irrs));

    bool all_unique_local = true;
    if (local_size > 1) { // If there is just one, then it is unique
      if (irss[0].rank == irss[1].rank) {
        irss[0].state = rank_state::NONE;
      }
      for (size_t i = 1; i + 1 < local_size; ++i) {
        if (irss[i].rank == irss[i - 1].rank ||
            irss[i].rank == irss[i + 1].rank) {
          irss[i].state = rank_state::NONE;
          all_unique_local = false;
        }
      }
      if (irss[local_size - 1].rank == irss[local_size - 2].rank) {
        irss[local_size - 1].state = rank_state::NONE;
        all_unique_local = false;
      }
    }

    if constexpr (debug) {
      size_t local_discarded = fully_discarded.size();
      size_t local_unique = unique.size();
      size_t local_undecided = irss.size();

      size_t global_discarded = dsss::mpi::allreduce_sum(local_discarded);
      size_t global_unique = dsss::mpi::allreduce_sum(local_unique);
      size_t global_undecided = dsss::mpi::allreduce_sum(local_undecided);

      if (env.rank() == 0) {
        std::cout << "Global discarded " << global_discarded << std::endl
                  << "Global unique " << global_unique << std::endl
                  << "Global undecided " << global_undecided << std::endl
                  << "Total " << global_discarded + global_unique + 
                                 global_undecided
                  << std::endl;
      }
      env.barrier();
    }

    bool all_unique_global = dsss::mpi::allreduce_and(all_unique_local, env);
    if (all_unique_global) {
      break;
    }

    irss = dsss::mpi::distribute_data(irss, env);
 
    if constexpr (debug_time_only || debug) {
       env.barrier();
       auto end_time = MPI_Wtime();
      if (env.rank() == 0) {
        std::cout << "Finished iteration " << iteration << " (with discarding)"
                  << " in " << end_time - start_time << " seconds"
                  << std::endl;
      }
    }

    ++iteration;

  }
  fully_discarded.reserve(fully_discarded.size() + irss.size());
  std::transform(irss.begin(), irss.end(),
    std::back_inserter(fully_discarded),
    [](const IRS& irs) {
      return IR { irs.index, irs.rank };
    });
  
  if constexpr (return_isa) {
    dsss::mpi::sort(fully_discarded, mpi_ir_type, [](const IR& a, const IR& b) {
      return a.index < b.index;
    }, env);
  } else {
    dsss::mpi::sort(fully_discarded, mpi_ir_type, [](const IR& a, const IR& b) {
      return a.rank < b.rank;
    }, env);
  }
  
  std::vector<IndexType> result;
  result.reserve(fully_discarded.size());
  std::transform(fully_discarded.begin(), fully_discarded.end(),
    std::back_inserter(result),
    [](const IR& ir) {
      if constexpr (return_isa) {
        return ir.rank;
      } else {
        return ir.index;
      }
    });

  return result;
}

template <typename IndexType, bool return_isa = false>
std::vector<IndexType> prefix_doubling_discarding(
  dsss::distributed_string&& distributed_raw_string) {

  using IR = index_rank<IndexType>;
  using IRS = index_rank_state<IndexType>;
  using IRR = index_rank_rank<IndexType>;

  MPI_Datatype mpi_index_type;
  MPI_Type_contiguous(sizeof(IndexType), MPI_CHAR, &mpi_index_type);
  MPI_Type_commit(&mpi_index_type);

  constexpr size_t ir_num_members = 2;
  int32_t ir_lengths[ir_num_members] = { 1, 1 };
  MPI_Aint ir_offsets[ir_num_members] = { offsetof(IR, index),
                                          offsetof(IR, rank) };
  MPI_Datatype ir_types[ir_num_members] = { mpi_index_type, mpi_index_type };

  MPI_Datatype mpi_ir_type;
  MPI_Type_create_struct(ir_num_members, ir_lengths, ir_offsets, ir_types,
                         &mpi_ir_type);
  MPI_Type_commit(&mpi_ir_type);

  constexpr size_t irr_num_members = 3;
  int32_t irr_lengths[irr_num_members] = { 1, 1, 1 };
  MPI_Aint irr_offsets[irr_num_members] = { offsetof(IRR, index),
                                            offsetof(IRR, rank1),
                                            offsetof(IRR, rank2) };
  MPI_Datatype irr_types[irr_num_members] = { mpi_index_type, mpi_index_type,
                                              mpi_index_type };

  MPI_Datatype mpi_irr_type;
  MPI_Type_create_struct(irr_num_members, irr_lengths, irr_offsets, irr_types,
                         &mpi_irr_type);
  MPI_Type_commit(&mpi_irr_type);

  dsss::mpi::environment env;

  size_t offset = 0;
  size_t local_size = 0;
  size_t iteration = 0;
  std::vector<IRR> irrs = pack_alphabet<IndexType>(distributed_raw_string,
                                                   iteration);
  std::vector<IR> irs;
  // Start one round of prefix doubling

  dsss::mpi::sort(irrs, mpi_irr_type, [](const IRR& a, const IRR& b) {
    return std::tie(a.rank1, a.rank2) < std::tie(b.rank1, b.rank2); },
    env);

  // Compute new ranks
  local_size = irrs.size();
  offset = dsss::mpi::ex_prefix_sum(local_size);

  // irs.clear();
  // irs.reserve(local_size);

  std::remove_reference_t<decltype(irs)>(std::move(irs));

  size_t cur_rank = offset;
  irs.emplace_back(irrs[0].index, cur_rank);
  for (size_t i = 1; i < local_size; ++i) {
    if (irrs[i - 1] != irrs[i]) {
      cur_rank = offset + i;
    }
    irs.emplace_back(irrs[i].index, cur_rank);
  }

  std::remove_reference_t<decltype(irrs)>(std::move(irrs));

  bool all_distinct = true;
  for (size_t i = 1; i < irs.size(); ++i) {
    all_distinct &= (irs[i].rank != irs[i - 1].rank);
    if (!all_distinct) {
      break;
    }
  }

  const bool finished = dsss::mpi::allreduce_and(all_distinct, env);
  if (finished) { 
    std::vector<IndexType> result;
    result.reserve(irs.size());
    std::transform(irs.begin(), irs.end(), std::back_inserter(result),
                   [](const IR& ir) { return ir.index; });
    return result;
  }

  dsss::mpi::sort(irs, mpi_ir_type, [iteration](const IR& a, const IR& b) {
    IndexType const mod_mask = (size_t(1) << iteration) - 1;
    IndexType const div_mask = ~mod_mask;

    if ((a.index & mod_mask) == (b.index & mod_mask)) {
      return (a.index & div_mask) < (b.index & div_mask);
    } else {
      return (a.index & mod_mask) < (b.index & mod_mask);
    }
  }, env);

  local_size = irs.size();
  offset = dsss::mpi::ex_prefix_sum(local_size, env);

  IR rightmost_ir = dsss::mpi::shift_left(irs.front(), env);
  if (env.rank() + 1 < env.size()) {
    irs.emplace_back(rightmost_ir);
  } else {
    irs.emplace_back(0, 0);
  }

  irrs.reserve(local_size);
  
  const size_t index_distance = size_t(1) << iteration;
  for (size_t i = 0; i < local_size; ++i) {
    IndexType second_rank = { 0 };
    if (DSSS_LIKELY(irs[i].index + index_distance == irs[i + 1].index)) {
      second_rank = irs[i + 1].rank;
    }
    irrs.emplace_back(irs[i].index, irs[i].rank, second_rank);
  }

  // irs.clear();
  // irs.shrink_to_fit();
  std::remove_reference_t<decltype(irs)>(std::move(irs));

  dsss::mpi::sort(irrs, mpi_irr_type, [](const IRR& a, const IRR& b) {
    return std::tie(a.rank1, a.rank2) < std::tie(b.rank1, b.rank2);
  }, env);

  local_size = irrs.size();
  std::vector<IRS> irss;
  irss.reserve(local_size);
  offset = dsss::mpi::ex_prefix_sum(local_size, env) + 1;
  cur_rank = offset;
  irss.emplace_back(irrs[0].index, cur_rank, rank_state::NONE);
  for (size_t i = 1; i < local_size; ++i) {
    if (irrs[i - 1] != irrs[i]) {
      cur_rank = offset + i;
    }
    irss.emplace_back(irrs[i].index, cur_rank, rank_state::NONE);
  }
  if (irss.size() == 1) {
    irss[0].state = rank_state::UNIQUE;
  } else if (irss.size() > 1) {
    irss[0].state = (irss[0].rank != irss[1].rank) ?
      rank_state::UNIQUE : rank_state::NONE;
    for (size_t i = 1; i + 1 < local_size; ++i) {
      if (irss[i].rank != irss[i - 1].rank &&
          irss[i].rank != irss[i + 1].rank) {
        irss[i].state = rank_state::UNIQUE;
      }
    }
    irss[local_size - 1].state =
      (irss[local_size - 1].rank != irss[local_size - 2].rank) ?
        rank_state::UNIQUE : rank_state::NONE;
  }

  // irrs.clear();
  // irrs.shrink_to_fit();

  std::remove_reference_t<decltype(irrs)>(std::move(irrs));

  if constexpr (debug) {
    env.barrier();
    if (env.rank() == 0) {
      std::cout << "Finished iteration " << iteration << " (w/o discarding)"
                << std::endl;
    }
    env.barrier();
  }
  ++iteration;
  return doubling_discarding<IndexType, return_isa>(irss, iteration, env);
}

} // namespace dsss::suffix_sorting

/******************************************************************************/
