/*******************************************************************************
 * benchmark/dsaca.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <tlx/cmdline_parser.hpp>

#include "mpi/allreduce.hpp"
#include "mpi/distribute_input.hpp"
#include "mpi/environment.hpp"

#include "suffix_sorting/prefix_doubling.hpp"
#include "suffix_sorting/sa_check.hpp"

#include "util/random_string_generator.hpp"
#include "util/string.hpp"
#include "util/uint_types.hpp"

size_t string_size = { 0 };
std::string input_path = "";
std::string output_path = "";
bool check = false;
bool doubling_discarding = true;

int32_t main(int32_t argc, char const *argv[]) {
  dsss::mpi::environment env;
  tlx::CmdlineParser cp;

  using index_type = dsss::uint40;
  // using index_type = size_t;

  cp.set_description("Distributed Suffix Array Construction");
  cp.set_author("Florian Kurpicz <florian.kurpicz@tu-dortmund.de>");

  cp.add_param_string("input", input_path,
                      "Path to input file. The special input 'random' generates"
                      " a random text of the size given by parameter '-s'.");

  cp.add_bytes('s', "size", string_size, "Size (in bytes unless stated "
               "otherwise) of the string that use to test our suffix array "
               "construction algorithms.");

  cp.add_flag('c', "check", check, "Check if the SA has been constructed "
              "correctly. This does not work with random text (no way to "
              " reproduce).");

  cp.add_string('o', "output", "<F>", output_path, "Filename for the output "
                "(SA). Note that the output is five times larger than the input"
                " file.");

  cp.add_flag('d', "discarding", doubling_discarding, "Compute the suffix array"
              " using prefix doubling with discarding (instead of without).");

  if (!cp.process(argc, argv)) {
    return -1;
  }

  dsss::distributed_string distributed_strings;

  if (!input_path.compare("random")) {
    string_size /= env.size();
    dsss::random_indexed_string_set<index_type> rss(string_size, 255);
    distributed_strings = { env.rank() * string_size,
      std::move(rss.data_container()) };
  } else {
    if (string_size > 0) {
      distributed_strings = dsss::mpi::distribute_string(
        input_path, string_size);
    } else {
      distributed_strings = dsss::mpi::distribute_string(input_path);
    }
  }
  std::vector<index_type> sa;
  auto start_time = MPI_Wtime();
  if (doubling_discarding) {
    sa = dsss::suffix_sorting::prefix_doubling_discarding<index_type>(
           std::move(distributed_strings));
  } else /*without*/ {
    sa = dsss::suffix_sorting::prefix_doubling<index_type>(
           std::move(distributed_strings));
  }
  auto end_time = MPI_Wtime();

  if (env.rank() == 0) {
    std::cout << "TIME: " << end_time - start_time << std::endl;
  }

  if (!output_path.empty()) {
    if (env.rank() == 0) {
      std::cout <<"Writing the SA to " << output_path << std::endl;
    }
    dsss::mpi::write_data(sa, output_path);
    env.barrier();
    if (env.rank() == 0) {
      std::cout << "Finished writing the SA" << std::endl;
    }
  }

  if (check) {
    if (string_size > 0) {
      distributed_strings = dsss::mpi::distribute_string(input_path,
                                                         string_size);
    } else {
      distributed_strings = dsss::mpi::distribute_string(input_path);
    }

    if (!output_path.empty()) {
      if (env.rank() == 0) {
        std::cout << "To check if export of the SA was successful, we first "
                  << "load the exported file ... ";
      }
      sa = dsss::mpi::read_data<index_type>(output_path);
      if (env.rank() == 0) {
        std::cout << "DONE" << std::endl;
      }
    }
    if (env.rank() == 0) { std::cout << "Checking SA ... "; }
    bool correct = dsss::suffix_sorting::check(sa, distributed_strings.string);
    if (!correct && env.rank() == 0) {
      std::cout << "ERROR: Not a correct SA!" << std::endl;
    } else if (env.rank() == 0) {
      std::cout << "Correct SA!" << std::endl;
    }
  }

  env.finalize();
  return 0;
}

/******************************************************************************/
