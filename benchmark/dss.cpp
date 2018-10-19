/*******************************************************************************
 * benchmark/dss.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <tlx/cmdline_parser.hpp>

#include <cstdint>
#include <iostream>

#include "mpi/allreduce.hpp"
#include "mpi/distribute_input.hpp"
#include "mpi/environment.hpp"
#include "mpi/shift.hpp"

#include "string_sorting/util/algorithm.hpp"
#include "string_sorting/distributed/sample_sort.hpp"
#include "suffix_sorting/classification.hpp"

std::size_t string_size;
std::string input_path;
bool b_star_substrings;
bool check;
bool export_times;

std::int32_t main(std::int32_t argc, char const *argv[]) {
  dsss::mpi::environment env;
  tlx::CmdlineParser cp;

  cp.set_description("A benchmark for sorting strings"
    " various sequential string sorters within our distributed string sorting."
    " algorithms.");
  cp.set_author("Florian Kurpicz <florian.kurpicz@tu-dortmund.de>");

  cp.add_param_string("input", input_path,
    "Path to input file. The special input 'random' generates a random text of"
    "the size given by parameter '-s'.");

  cp.add_flag('b', "b_start_substrings", b_star_substrings, "Compute the "
    "B*-substrings and sort them (if this option is chosen, there should be"
    "no null bytes in the text to split 'other' substrings.");

  cp.add_bytes('s', "size", string_size, "Size (in bytes unless stated "
    "otherwise) of the string that use to test our string sorting algorithms.");

  cp.add_flag('c', "check", check, "Check if the substrings have been sorted "
    "correctly.");

  if (!cp.process(argc, argv)) {
    return -1;
  }
  if (env.rank() == 0) {
    std::cout << "Distributed String Sorting" << std::endl;
  }
  env.barrier();

  auto& algorithm_list = dsss::algorithm_list::get_algorithm_list();
  for (const auto& algorithm : algorithm_list) {
    if (algorithm->category() == dsss::algorithm_category::DISTRIBUTED) {
      if (env.rank() == 0) {
        algorithm->print_info();
      }

      dsss::string_set strings_to_sort;
      if (b_star_substrings) {
        auto distributed_strings =
          dsss::mpi::distribute_string(input_path, string_size * env.size());
        std::tie(strings_to_sort, std::ignore) = dsss::suffix_sorting::
          b_star_substrings<std::size_t>(distributed_strings);
      } else {
        auto distributed_strings =
          dsss::mpi::distribute_strings(input_path, string_size * env.size());
        strings_to_sort =
          dsss::string_set(std::move(distributed_strings.string));
      }

      std::size_t local_size = strings_to_sort.size();
      std::size_t local_length = strings_to_sort.data_container().size();

      env.barrier();
      auto start_time = MPI_Wtime();
      algorithm->run(strings_to_sort);
      auto end_time = MPI_Wtime();
      if (env.rank() == 0) {
        std::cout << "Running time: " << end_time - start_time << " seconds."
                  << std::endl;
      }
      env.barrier();
      if (check) {
        if (env.rank() == 0) {
          std::cout << "Checking correctness ... ";
        }
        bool all_sorted = true;

        auto smaller_string =
          dsss::mpi::shift_string_right(strings_to_sort.back());
        auto larger_string =
          dsss::mpi::shift_string_left(strings_to_sort.front());

        if (env.rank() > 0) {
          all_sorted &= dsss::string_smaller_eq(
            smaller_string.data(), strings_to_sort.front());
        }
        if (env.rank() + 1 < env.size()) {
          all_sorted &= dsss::string_smaller_eq(
            strings_to_sort.back(), larger_string.data());
        }

        for (std::size_t i = 0; i + 1 < strings_to_sort.size(); ++i) {
          all_sorted &= dsss::string_smaller_eq(
            strings_to_sort[i], strings_to_sort[i + 1]);
        }

        all_sorted = dsss::mpi::allreduce_and(all_sorted);

        if (!all_sorted) {
          std::cout << "ERROR: strings not sorted correctly at PE "
                    << env.rank() << "." << std::endl;
          std::exit(-1);
        }

        std::size_t initial_global_size = dsss::mpi::allreduce_sum(local_size);
        std::size_t initial_global_length = dsss::mpi::allreduce_sum(local_length);
        local_size = strings_to_sort.size();
        local_length = strings_to_sort.data_container().size();
        std::size_t new_global_size = dsss::mpi::allreduce_sum(local_size);
        std::size_t new_global_length = dsss::mpi::allreduce_sum(local_length);

        if (env.rank() == 0) {
          if (initial_global_size == new_global_size &&
            initial_global_length == new_global_length) {
            std::cout << "everything OK!" << std::endl;
          } else if (initial_global_length < new_global_length) {
            std::cout << "ERROR: some characters appeared out of nowhere."
                      << std::endl;
            std::exit(-1);
          } else if (initial_global_size < new_global_size) {
            std::cout << "ERROR: some strings appeared out of nowhere."
                      << std::endl;
            std::exit(-1);
           } else if (initial_global_length > new_global_length) {
            std::cout << "ERROR: we lost some characters during the sorting."
                      << std::endl;
            std::exit(-1);
           } else {
            std::cout << "ERROR: we lost some strings during the sorting."
                      << std::endl;
            std::exit(-1);
          }
        }
      }
    }
  }

  env.barrier();
  env.finalize();
  return 0;
}

/******************************************************************************/
