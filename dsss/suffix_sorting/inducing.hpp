/*******************************************************************************
 * suffix_sorting/dsaca.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/broadcast.hpp"
#include "mpi/environment.hpp"
#include "mpi/gather.hpp"
#include "mpi/induce.hpp"
#include "mpi/requestable_array.hpp"
#include "mpi/scan.hpp"
#include "mpi/scatter.hpp"
#include "mpi/sort.hpp"
#include "mpi/shift.hpp"
#include "mpi/zip.hpp"
#include "sequential/bingmann-radix_sort.hpp"
#include "string_sorting/distributed/merge_sort.hpp"
#include "string_sorting/sequential/indexed_radix_sort.hpp"
#include "suffix_sorting/classification.hpp"
#include "suffix_sorting/data_structs.hpp"
#include "suffix_sorting/prefix_doubling.hpp"
#include "util/macros.hpp"
#include "util/string.hpp"

namespace dsss::suffix_sorting {

template <typename IndexType>
inline void sort_bs_substrings(
  dsss::indexed_string_set<IndexType>& bs_substrings,
  [[maybe_unused]] dsss::mpi::environment env = dsss::mpi::environment()) {

  // Sort the indexed input. After this step, we know that the substrings stored
  // at other PEs are strictly smaller or greater. We use this property to set
  // the initial rank in our inverse suffix array.
  dsss::sample_sort::sample_sort<IndexType, dsss::msd_CE0<IndexType>,
    bingmann::bingmann_msd_CE3>(bs_substrings);
}

template <typename IndexType>
std::vector<IndexType> sort_bs_suffixes(
  dsss::indexed_string_set<IndexType>& bs_substrings,
  dsss::mpi::environment env = dsss::mpi::environment()) {

  using IR = index_rank<IndexType>;
  using IRR = index_rank_rank<IndexType>;
  using IRS = index_rank_state<IndexType>;

  size_t local_size = bs_substrings.size();

  size_t offset = dsss::mpi::ex_prefix_sum(local_size);
  IndexType cur_rank = IndexType(offset);

  std::vector<IR> irs;
  irs.reserve(local_size);

  if (local_size > 0) {
    irs.emplace_back(bs_substrings[0].index, cur_rank);
  }
  for (size_t i = 1; i < local_size; ++i) {
    if (!dsss::string_eq(bs_substrings[i].string,
                         bs_substrings[i - 1].string)) {
      cur_rank = IndexType(offset + i);
    }
    irs.emplace_back(bs_substrings[i].index, cur_rank);
  }

  bool all_distinct = true;
  for (size_t i = 1; i < irs.size(); ++i) {
    all_distinct &= (irs[i].rank != irs[i - 1].rank);
    if (!all_distinct) {
      break;
    }
  }
  bool finished = dsss::mpi::allreduce_and(all_distinct, env);

  std::vector<IndexType> bs_positions;
  if (finished) {
    // Everything is sorted before we have done anything    
    bs_positions.reserve(local_size);
    std::transform(irs.begin(), irs.end(), std::back_inserter(bs_positions),
                   [](const IR& a) { return a.index; });
    return bs_positions;
  }

  dsss::mpi::sort(irs, [](const IR& a, const IR& b) {
                         return a.index < b.index; }, env);

  local_size = irs.size();
  offset = dsss::mpi::ex_prefix_sum(local_size);

  bs_positions.reserve(local_size);
  IndexType string_pos = IndexType(offset);
  for (size_t i = 0; i < irs.size(); ++i) {
    bs_positions.emplace_back(irs[i].index);
    irs[i].index = ++string_pos;
  }

  size_t iteration = 0;
  dsss::mpi::sort(irs, [iteration](const IR& a, const IR& b) {
    IndexType mod_mask = (size_t(1) << iteration) - 1;
    IndexType div_mask = ~mod_mask;

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

  std::vector<IRR> irrs;
  irrs.reserve(local_size);
  const size_t index_distance = size_t(1) << iteration;
  for (size_t i = 0; i < local_size; ++i) {
    IndexType second_rank = { 0 };
    if (DSSS_LIKELY(irs[i].index + index_distance == irs[i + 1].index)) {
      second_rank = irs[i + 1].rank;
    }
    irrs.emplace_back(irs[i].index, irs[i].rank, second_rank);
  }

  irs.clear();
  irs.shrink_to_fit();

  dsss::mpi::sort(irrs, [](const IRR& a, const IRR& b) {
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
  }
  else if (irss.size() > 1) {
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

  irrs.clear();
  irrs.shrink_to_fit();

  ++iteration;
  auto part_isa = doubling_discarding<IndexType, true>(irss, iteration, env);
  
  irs = dsss::mpi::zip(bs_positions, part_isa,
    [](const IndexType idx, const IndexType isa) {
      return IR { idx, isa };
    },
  env);

  part_isa.clear();
  part_isa.shrink_to_fit();
  bs_positions.clear();
  bs_positions.shrink_to_fit();

  dsss::mpi::sort(irs, [](const IR& a, const IR& b) {
    return a.rank < b.rank;
  }, env);

  std::vector<IndexType> result;
  result.reserve(irs.size());
  std::transform(irs.begin(), irs.end(), std::back_inserter(result),
    [](const IR& ir) { return ir.index; });

  return result;
}

template <bool is_right_to_left = false>
inline size_t compute_local_size(size_t const global_size,
  dsss::mpi::environment env = dsss::mpi::environment()) {
  size_t local_size = 0;
  if (global_size < size_t(env.size())) {
    if constexpr (is_right_to_left) {
      if (env.rank() == 0) {
        local_size = global_size;
      }
    } else /*if (!is_right_to_left)*/{
      if (env.rank() + 1 == env.size()) {
        local_size = global_size;
      }
    }
  } else {
    local_size = global_size / env.size();
    if constexpr (is_right_to_left) {
      if (env.rank() == 0) {
        local_size += (global_size % env.size());
      }
    } else /*if (!is_right_to_left)*/ {
      if (env.rank() + 1 == env.size()) {
        local_size += (global_size % env.size());
      }
    }
  }
  return local_size;
}

template <typename IndexType>
std::vector<IndexType> inducing(dsss::distributed_string&& distributed_input) {
  using bucket_info = bucket_info<IndexType>;
  
  dsss::mpi::environment env;
  // 1. Classify string
  auto [ classified_strings, b_array ] = idx_b_star_substrings<IndexType>(
                                           distributed_input);

  distributed_input.string = dsss::mpi::distribute_data(distributed_input.string);
  size_t local_string_size = distributed_input.string.size();
  local_string_size = dsss::mpi::allreduce_sum(local_string_size);
  dsss::mpi::requestable_array req_text(distributed_input.string, local_string_size);

  // 2. Sort B*-substrings & B*-suffixes
  sort_bs_substrings(classified_strings);
  std::vector<IndexType> sorted_bs_suffixes =
    sort_bs_suffixes<IndexType>(classified_strings);

  constexpr size_t max_char = std::numeric_limits<dsss::char_type>::max();
  std::vector<bucket_info> a_buckets((max_char + 1) * (max_char + 1),
                                     { 0, 0, 0 });
  std::vector<bucket_info> b_buckets((max_char + 1) * (max_char + 1),
                                     { 0, 0, 0 });

  auto suffix_id = [max_char](const size_t c0, const size_t c1) {
    return c0 + ((max_char + 1) * c1);
  };

  auto star_suffix_id = [max_char](const size_t c0, const size_t c1) {
    return ((max_char + 1) * c0) + c1;
  };

  // 3. Prepare distributed arrays
  // 3.1 Create intervals that will define the distributed arrays
  size_t summed_size = 0;
  for (size_t c0 = 0; c0 <= max_char; ++c0) {
    size_t c1;
    size_t size;
    for (c1 = 0; c1 < c0; ++c1) {
      size = compute_local_size(b_array.a(c0, c1));
      // A-Buckets
      a_buckets[suffix_id(c0, c1)] = bucket_info { summed_size, size, 0 };
      summed_size += size;
      size = compute_local_size<true>(b_array.a_star(c0, c1));
      //A*-Buckets
      a_buckets[star_suffix_id(c0, c1)] = bucket_info { summed_size, size, 0 };
      summed_size += size;
    }
    // c0 == c1
    // A-Bucket (there is no A*-Bucket)
    size = compute_local_size(b_array.a(c0, c0));
    a_buckets[suffix_id(c0, c0)] = bucket_info { summed_size, size, 0 };
    summed_size += size;
    // B-Bucket (there is no B*-Bucket)
    size = compute_local_size<true>(b_array.b(c0, c0));
    b_buckets[suffix_id(c0, c0)] = bucket_info { summed_size, size, 0 };
    summed_size += size;
    for (c1 = c0 + 1; c1 <= max_char; ++c1) {
      size = compute_local_size<true>(b_array.b_star(c0, c1));
      //B*-Buckets
      b_buckets[star_suffix_id(c0, c1)] = bucket_info { summed_size, size, 0 };
      summed_size += size;
      size = compute_local_size<true>(b_array.b(c0, c1));
      // B-Buckets
      b_buckets[suffix_id(c0, c1)] = bucket_info { summed_size, size,0 };
      summed_size += size;
    }
  }


  // 3.2 Allocate local part of the SA
  std::vector<IndexType> local_sa(summed_size, IndexType(0));

  // 3.3 Fill B*-Buckets
  sorted_bs_suffixes = dsss::mpi::distribute_data(sorted_bs_suffixes, env);
  size_t bs_count = sorted_bs_suffixes.size();
  size_t bs_total_count = dsss::mpi::allreduce_sum(bs_count); // Total size

  size_t bs_slice_size = bs_total_count / env.size();
  size_t last_slice_offset = 0;
  size_t seen_bs = 0;
  int32_t cur_root = 0;
  for (size_t c0 = 0; c0 <= max_char; ++c0) {
    for (size_t c1 = c0 + 1; c1 <= max_char; ++c1) {
      if (size_t new_bs = b_array.b_star(c0, c1); new_bs > 0) {
        if (seen_bs + new_bs < (((cur_root + 1) * bs_slice_size) + last_slice_offset)) {
          dsss::mpi::scatter_distibuted(sorted_bs_suffixes.data() +
                                        seen_bs - (cur_root * bs_slice_size),
                                        new_bs,
                                        cur_root,
                                        local_sa.data() +
                                        b_buckets[star_suffix_id(c0, c1)].starting_position,
                                        env);

          b_buckets[star_suffix_id(c0, c1)].containing =
              b_buckets[star_suffix_id(c0, c1)].size;
          seen_bs += new_bs;
        } else {
          int32_t first_target = 0;
          size_t remaining_bs = new_bs;
          size_t local_bs_slice = new_bs / env.size();
          size_t first_bs_larger = new_bs % env.size();
          size_t still_required = 0;
          while (remaining_bs > 0) {
            size_t to_scatter = std::min(remaining_bs,
                                         (((cur_root + 1 ) * bs_slice_size) +
                                          last_slice_offset - seen_bs));
            size_t sent = 0;
            std::vector<size_t> send_counts(env.size(), 0);
            for (; first_target < env.size() && to_scatter > 0; ++first_target) {
              if (first_target > 0) {
                first_bs_larger = 0;
              }
              size_t slice = std::min(to_scatter,
                                      (still_required > 0 ? still_required :
                                       local_bs_slice + first_bs_larger));
              send_counts[first_target] = slice;
              to_scatter -= slice;
              sent += slice;
              still_required = 0;
            }
            if (send_counts[first_target - 1] < local_bs_slice) {
              still_required = local_bs_slice - send_counts[first_target - 1];
              --first_target;
            }
            if (to_scatter > 0) {
              send_counts.front() += to_scatter;
              sent += to_scatter;
            }

            size_t rcvd = dsss::mpi::scatterv(sorted_bs_suffixes.data() +
                                              seen_bs -
                                              (cur_root * bs_slice_size),
                                              sent,
                                              send_counts,
                                              cur_root,
                                              local_sa.data() +
                                              b_buckets[star_suffix_id(c0, c1)].front_pos(),
                                              env);

            b_buckets[star_suffix_id(c0, c1)].containing += rcvd;
            seen_bs += sent;
            remaining_bs -= sent;

            if (seen_bs == (cur_root + 1) * bs_slice_size) {
              ++cur_root;
              if (cur_root + 1 == env.size()) {
                last_slice_offset = bs_total_count % env.size();
              }
            }
          }
        }
      }
    }
  }

  // 4.1 Induce the B-Suffixes
  dsss::mpi::inducer<IndexType, max_char> ind_util(env);

  std::array<size_t, max_char + 1> borders;
  std::array<size_t, max_char + 1> hist;

  auto induce_b = [&](const size_t c0, const bucket_info& cur_bckt,
                      const size_t global_size) {
    if (global_size > 0) {
      std::vector<IndexType> req_pos;
      req_pos.reserve(cur_bckt.size);
      for (size_t i = 0; i < cur_bckt.size; ++i) {
        if (size_t val = local_sa[cur_bckt.starting_position + i];
            DSSS_LIKELY(val > 0)) {
          req_pos.push_back(val - 1);
        }
      }

      auto res_chars = req_text.request2(req_pos);
 
      std::fill_n(borders.begin(), max_char + 1, 0);
      std::fill_n(hist.begin(), max_char + 1, 0);
      for (size_t i = 0; i < res_chars.size(); ++i) {
        ++hist[res_chars[i]];
      }
      for (size_t i = 1; i < max_char + 1; ++i) {
        borders[i] = borders[i - 1] + hist[i - 1];
      }
        
      std::vector<IndexType> to_induce(res_chars.size());
      for (size_t i = 0; i < res_chars.size(); ++i) {
        to_induce[borders[res_chars[i]]++] = req_pos[i];
      }
      // Induce
      std::vector<IndexType*> start_positions;
      start_positions.reserve(max_char + 1);
      std::vector<size_t> small_hist;
      small_hist.reserve(max_char + 1);
      std::vector<size_t> local_containing;
      local_containing.reserve(max_char + 1);
      std::vector<size_t> global_sizes;
      global_sizes.reserve(max_char + 1);
      std::vector<IndexType*> cur_target_pos;
      cur_target_pos.reserve(max_char + 1);
      std::vector<bucket_info*> tar_buckets;
      tar_buckets.reserve(max_char + 1);

      auto global_hist = dsss::mpi::allreduce_sum(hist, env);
      size_t start_p = 0;
      for (size_t i = 0; i < max_char + 1; ++i) {
        if (global_hist[i] > 0) {
          bucket_info& tar_bckt = (i <= c0) ? b_buckets[suffix_id(i, c0)] :
            a_buckets[star_suffix_id(i, c0)];
          local_containing.push_back(tar_bckt.containing);
          cur_target_pos.push_back(local_sa.data() + tar_bckt.back_pos());
          global_sizes.push_back((i <= c0) ? b_array.b(i, c0) :
                                             b_array.a_star(i, c0));
          small_hist.push_back(hist[i]);
          start_positions.push_back(to_induce.data() + start_p);
          tar_buckets.push_back(&tar_bckt);
          start_p += hist[i];
        }
      }

      ind_util.induce_right_to_left(start_positions,
                                    small_hist,
                                    local_containing,
                                    global_sizes,
                                    cur_target_pos,
                                    tar_buckets);
    }
  };
  
  auto induce_b_special = [&](size_t const c0, bucket_info const& cur_bckt,
                              size_t const global_size) {
    if (global_size > 0) {
      bool completed = false;
      size_t cur_pos = 0;
      while (!completed) {
        std::vector<IndexType> req_pos;
        for (; cur_pos < cur_bckt.containing; ++cur_pos) {
          if (size_t val = local_sa[cur_bckt.starting_position +
                                    cur_bckt.size - IndexType(1) - cur_pos];
              DSSS_LIKELY(val > 0)) {
            req_pos.push_back(val - 1);
          }
        }
        std::reverse(req_pos.begin(), req_pos.end());
        
        auto res_chars = req_text.request2(req_pos);

        std::fill_n(borders.begin(), max_char + 1, 0);
        std::fill_n(hist.begin(), max_char + 1, 0);
        for (size_t i = 0; i < req_pos.size(); ++i) {
          ++hist[res_chars[i]];
        }
        for (size_t i = 1; i < max_char + 1; ++i) {
          borders[i] = borders[i - 1] + hist[i - 1];
        }
        
        std::vector<IndexType> to_induce(res_chars.size());
        for (size_t i = 0; i < res_chars.size(); ++i) {
          to_induce[borders[res_chars[i]]++] = req_pos[i];
        }

        // Induce
        std::vector<IndexType*> start_positions;
        start_positions.reserve(max_char + 1);
        std::vector<size_t> small_hist;
        small_hist.reserve(max_char + 1);
        std::vector<size_t> local_containing;
        local_containing.reserve(max_char + 1);
        std::vector<size_t> global_sizes;
        global_sizes.reserve(max_char + 1);
        std::vector<IndexType*> cur_target_pos;
        cur_target_pos.reserve(max_char + 1);
        std::vector<bucket_info*> tar_buckets;
        tar_buckets.reserve(max_char + 1);

        auto global_hist = dsss::mpi::allreduce_sum(hist, env);
        size_t start_p = 0;
        for (size_t i = 0; i < max_char + 1; ++i) {
          if (global_hist[i] > 0) {
            bucket_info& tar_bckt = (i <= c0) ? b_buckets[suffix_id(i, c0)] :
              a_buckets[star_suffix_id(i, c0)];
            local_containing.push_back(tar_bckt.containing);
            cur_target_pos.push_back(local_sa.data() + tar_bckt.back_pos());
            global_sizes.push_back((i <= c0) ? b_array.b(i, c0) :
                                               b_array.a_star(i, c0));
            small_hist.push_back(hist[i]);
            start_positions.push_back(to_induce.data() + start_p);
            tar_buckets.push_back(&tar_bckt);
            start_p += hist[i];
          }
        }

        ind_util.induce_right_to_left(start_positions,
                                      small_hist,
                                      local_containing,
                                      global_sizes,
                                      cur_target_pos,
                                      tar_buckets);

        bool tmp_completed = (cur_bckt.size == IndexType(cur_pos));
        completed = dsss::mpi::allreduce_and(tmp_completed);
      }
    }
  };

  auto induce_a = [&](const size_t c0, const bucket_info& cur_bckt,
                      const size_t global_size) {
    if (global_size > 0) {
      std::vector<IndexType> req_pos;
      req_pos.reserve(cur_bckt.size);
      for (size_t i = 0; i < cur_bckt.size; ++i) {
        if (size_t val = local_sa[cur_bckt.starting_position + i];
            DSSS_LIKELY(val > 0)) {
          req_pos.push_back(val - 1);
        }
      }
      auto res_chars = req_text.request2(req_pos);

      std::fill_n(borders.begin(), max_char + 1, 0);
      std::fill_n(hist.begin(), max_char + 1, 0);
      for (size_t i = 0; i < req_pos.size(); ++i) {
        ++hist[res_chars[i]];
      }
      for (size_t i = 1; i < max_char + 1; ++i) {
        borders[i] = borders[i - 1] + hist[i - 1];
      }
        
      std::vector<IndexType> to_induce(res_chars.size());
      for (size_t i = 0; i < res_chars.size(); ++i) {
        to_induce[borders[res_chars[i]]++] = req_pos[i];
      }


      std::vector<IndexType*> start_positions;
      start_positions.reserve(max_char + 1);
      std::vector<size_t> small_hist;
      small_hist.reserve(max_char + 1);
      std::vector<size_t> local_containing;
      local_containing.reserve(max_char + 1);
      std::vector<size_t> global_sizes;
      global_sizes.reserve(max_char + 1);
      std::vector<IndexType*> cur_target_pos;
      cur_target_pos.reserve(max_char + 1);
      std::vector<bucket_info*> tar_buckets;
      tar_buckets.reserve(max_char + 1);

      auto global_hist = dsss::mpi::allreduce_sum(hist, env);
      size_t start_p = (c0 == 0) ? 0 : borders[c0 - 1];;
      for (size_t i = c0; i < max_char + 1; ++i) {
        if (global_hist[i] > 0) {
          bucket_info& tar_bckt = a_buckets[suffix_id(i, c0)];
          local_containing.push_back(tar_bckt.containing);
          cur_target_pos.push_back(local_sa.data() + tar_bckt.front_pos());
          global_sizes.push_back(b_array.a(i, c0));
          small_hist.push_back(hist[i]);
          start_positions.push_back(to_induce.data() + start_p);
          tar_buckets.push_back(&tar_bckt);
          start_p += hist[i];
        }
      }

      ind_util.induce_left_to_right(start_positions,
                                    small_hist,
                                    local_containing,
                                    global_sizes,
                                    cur_target_pos,
                                    tar_buckets);
    }
  };

  auto induce_a_special = [&](size_t const c0, bucket_info const& cur_bckt,
                              size_t const global_size) {
    if (global_size > 0) {
      bool completed = false;
      size_t cur_pos = 0;
      while (!completed) {
        std::vector<IndexType> req_pos;
        for (; cur_pos < cur_bckt.containing; ++cur_pos) {
          if (size_t val = local_sa[cur_bckt.starting_position + cur_pos];
              DSSS_LIKELY(val > 0)) {
            req_pos.push_back(val - 1);
          }
        }
        auto res_chars = req_text.request2(req_pos);

        std::fill_n(borders.begin(), max_char + 1, 0);
        std::fill_n(hist.begin(), max_char + 1, 0);
        for (size_t i = 0; i < req_pos.size(); ++i) {
          ++hist[res_chars[i]];
        }
        for (size_t i = 1; i < max_char + 1; ++i) {
          borders[i] = borders[i - 1] + hist[i - 1];
        }
        
        std::vector<IndexType> to_induce(res_chars.size());
        for (size_t i = 0; i < res_chars.size(); ++i) {
          to_induce[borders[res_chars[i]]++] = req_pos[i];
        }

        std::vector<IndexType*> start_positions;
        start_positions.reserve(max_char + 1);
        std::vector<size_t> small_hist;
        small_hist.reserve(max_char + 1);
        std::vector<size_t> local_containing;
        local_containing.reserve(max_char + 1);
        std::vector<size_t> global_sizes;
        global_sizes.reserve(max_char + 1);
        std::vector<IndexType*> cur_target_pos;
        cur_target_pos.reserve(max_char + 1);
        std::vector<bucket_info*> tar_buckets;
        tar_buckets.reserve(max_char + 1);

        auto global_hist = dsss::mpi::allreduce_sum(hist, env);
        size_t start_p = (c0 == 0) ? 0 : borders[c0 - 1];;
        for (size_t i = c0; i < max_char + 1; ++i) {
          if (global_hist[i] > 0) {
            bucket_info& tar_bckt = a_buckets[suffix_id(i, c0)];
            local_containing.push_back(tar_bckt.containing);
            cur_target_pos.push_back(local_sa.data() + tar_bckt.front_pos());
            global_sizes.push_back(b_array.a(i, c0));
            small_hist.push_back(hist[i]);
            start_positions.push_back(to_induce.data() + start_p);
            tar_buckets.push_back(&tar_bckt);
            start_p += hist[i];
          }
        }

        ind_util.induce_left_to_right(start_positions,
                                      small_hist,
                                      local_containing,
                                      global_sizes,
                                      cur_target_pos,
                                      tar_buckets);

        bool tmp_completed = (cur_bckt.size == IndexType(cur_pos));
        completed = dsss::mpi::allreduce_and(tmp_completed);
      }
    }
  };

  for (size_t c0 = max_char; c0 ; --c0) {
    for (size_t c1 = max_char - 1; c1 > c0; --c1) {
      // Induce from B-bucket
      induce_b(c0, b_buckets[suffix_id(c0, c1)], b_array.b(c0, c1));
      // Induce from B*-bucket
      induce_b(c0, b_buckets[star_suffix_id(c0, c1)], b_array.b_star(c0, c1));
    }
    // SPECIAL CASE
    induce_b_special(c0, b_buckets[suffix_id(c0, c0)], b_array.b(c0, c0));
  }

  // 4.2 Put the last suffix at its correct position
  const size_t total = dsss::mpi::allreduce_sum(summed_size);
  auto last_char = dsss::mpi::broadcast(req_text.back(), env.size() - 1, env);

  if (env.rank() == 0) {
    local_sa[a_buckets[star_suffix_id(last_char, 0)].starting_position] = total - 1;
    a_buckets[star_suffix_id(last_char, 0)].containing = 1;
  }

  // 4.3 Induce the A-Suffixes
  for (size_t c0 = 0; c0 < max_char + 1; ++c0) {
    for (size_t c1 = 0; c1 < c0; ++c1) {
      // Induce from A-bucket
      induce_a(c0, a_buckets[suffix_id(c0, c1)], b_array.a(c0, c1));
      // Induce from A*-bucket
      induce_a(c0, a_buckets[star_suffix_id(c0, c1)], b_array.a_star(c0, c1));
    }
    // SPECIAL CASE
    induce_a_special(c0, a_buckets[suffix_id(c0, c0)], b_array.a(c0, c0));
  }
  
  // 5. Reorder local_sa to contain the local slice of the SA, not the
  //    distrubted arrays
  size_t slice_size = total / env.size();
  last_slice_offset = 0;
  size_t local_size = slice_size + ((env.rank() + 1 == env.size()) ? total % env.size() : 0);
  std::vector<IndexType> sa(local_size, 0);

  int32_t cur_target = 0;
  size_t seen_pos = 0;

  auto gather_sa = [&](size_t const new_pos, bucket_info const& cur_bucket) {    
    if (new_pos > 0) {
      if (seen_pos + new_pos < ((cur_target + 1) * slice_size + last_slice_offset)) {
        dsss::mpi::gatherv(local_sa.data() + 
                           cur_bucket.starting_position,
                           cur_bucket.size,
                           cur_target,
                           sa.data() + seen_pos - (cur_target * slice_size),
                           env);
        seen_pos += new_pos;
      } else {
        size_t remaining_pos = new_pos;
        size_t local_containing = cur_bucket.size;
        size_t const start_pos = dsss::mpi::ex_prefix_sum(local_containing, env);
        size_t const end_pos = start_pos + local_containing;
        size_t global_sent = 0;
        while (remaining_pos > 0) {
          size_t to_gather = std::min(remaining_pos,
                                      (cur_target + 1) * slice_size +
                                      last_slice_offset - seen_pos);
          size_t const send_start = std::max(start_pos, global_sent);
          size_t const send_end = std::min(end_pos, to_gather + global_sent);
          size_t const send_count = (send_start < send_end) ? 
            send_end - send_start : 0;

          dsss::mpi::gatherv(local_sa.data() +
                             cur_bucket.starting_position +
                             send_start - start_pos,
                             send_count,
                             cur_target,
                             sa.data() + seen_pos - (cur_target * slice_size),
                             env);

          global_sent += to_gather;
          remaining_pos -= to_gather;
          seen_pos += to_gather;

          if (seen_pos == (cur_target + 1) * slice_size) {
            ++cur_target;
            if (cur_target + 1 == env.size()) {
              last_slice_offset = total % env.size();
            }
          }
        }
      }
    }
  };

  for (size_t c0 = 0; c0 <= max_char; ++c0) {
    for (size_t c1 = 0; c1 < c0; ++c1) {
      // Gather A-Suffixes
      gather_sa(b_array.a(c0, c1), a_buckets[suffix_id(c0, c1)]);
      // Gather A*-Suffixes
      gather_sa(b_array.a_star(c0, c1), a_buckets[star_suffix_id(c0, c1)]);
    }
    // c0 == c1
    gather_sa(b_array.a(c0, c0), a_buckets[suffix_id(c0, c0)]);
    gather_sa(b_array.b(c0, c0), b_buckets[suffix_id(c0, c0)]);
    for (size_t c1 = c0 + 1; c1 <= max_char; ++c1) {
      // Gather B*-Suffixes
      gather_sa(b_array.b_star(c0, c1), b_buckets[star_suffix_id(c0, c1)]);
      // Gather B-Suffixes
      gather_sa(b_array.b(c0, c1), b_buckets[suffix_id(c0, c1)]);
    }
  }

  return sa;
}

} // namespace dsss::suffix_sorting

/******************************************************************************/
