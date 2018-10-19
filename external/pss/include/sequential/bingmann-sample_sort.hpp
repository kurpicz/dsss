/*******************************************************************************
 * src/sequential/bingmann-sample_sort.hpp
 *
 * Experiments with sequential Super Scalar String Sample-Sort (S^5).
 *
 *******************************************************************************
 * Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef PSS_SRC_SEQUENTIAL_BINGMANN_SAMPLE_SORT_HEADER
#define PSS_SRC_SEQUENTIAL_BINGMANN_SAMPLE_SORT_HEADER

#include "../tools/stringtools.hpp"
#include "../tools/timer_array.hpp"
#include "../tools/stats_writer.hpp"
#include "../tools/globals.hpp"
#include "../tools/lcgrandom.hpp"
#include "bingmann-radix_sort.hpp"

#include <tlx/string/hexdump.hpp>
#include <tlx/meta/log2.hpp>
#include <tlx/logger.hpp>

#include <algorithm>

namespace bingmann_sample_sort {

static const bool debug = false;
static const bool debug_splitter = false;
static const bool debug_bucketsize = false;
static const bool debug_recursion = false;
static const bool debug_splitter_tree = false;

using namespace stringtools;

typedef uint64_t key_type;

static const size_t l2cache = 256 * 1024;

static const unsigned DefaultTreebits = 10;

static const size_t g_samplesort_smallsort = 32 * 1024;

static const size_t oversample_factor = 2;

static const bool g_toplevel_only = false;

//! whether to instantiate tests for all tree classifier sizes
#define SAMPLE_SORT_EXPAND_VARIANTS      0

//! use ?: operator or if in tree descent
#define SSSS_TERNARY_OP 1

static size_t g_ss_steps, g_rs_steps;

enum {
    TM_GENERAL, TM_MAKE_SAMPLE, TM_MAKE_SPLITTER, TM_CLASSIFY,
    TM_PREFIXSUM, TM_PERMUTE, TM_SMALLSORT
};
static TimerArrayDummy g_timer(16);

static inline void sample_sort_pre()
{
    g_ss_steps = g_rs_steps = 0;
    g_timer.clear();
}

static inline void sample_sort_post()
{
    g_stats
    >> "toplevel_only" << g_toplevel_only
        >> "l2cache" << l2cache
        >> "steps_sample_sort" << g_ss_steps
        >> "steps_base_sort" << g_rs_steps;

    g_stats >> "tm_general" << g_timer.get(TM_GENERAL)
        >> "tm_make_sample" << g_timer.get(TM_MAKE_SAMPLE)
        >> "tm_make_splitter" << g_timer.get(TM_MAKE_SPLITTER)
        >> "tm_classify" << g_timer.get(TM_CLASSIFY)
        >> "tm_prefixsum" << g_timer.get(TM_PREFIXSUM)
        >> "tm_permute" << g_timer.get(TM_PERMUTE)
        >> "tm_smallsort" << g_timer.get(TM_SMALLSORT);
}

//! method called for recursively sorting "small" sets
static inline
void sample_sort_small_sort(string* strings, size_t n, size_t depth)
{
    bingmann::msd_CI(strings, n, depth);
}

// ---[ Implementations ]-------------------------------------------------------

void bingmann_sample_sortBSC(string* strings, size_t n);

void bingmann_sample_sortBTC(string* strings, size_t n);
void bingmann_sample_sortBTCA(string* strings, size_t n);
void bingmann_sample_sortBTCU(string* strings, size_t n);
void bingmann_sample_sortBTCU1(string* strings, size_t n);
void bingmann_sample_sortBTCU2(string* strings, size_t n);
void bingmann_sample_sortBTCU4(string* strings, size_t n);

void bingmann_sample_sortBTCE(string* strings, size_t n);
void bingmann_sample_sortBTCEA(string* strings, size_t n);

void bingmann_sample_sortBTCT(string* strings, size_t n);
void bingmann_sample_sortBTCTU(string* strings, size_t n);

} // namespace bingmann_sample_sort

#endif // !PSS_SRC_SEQUENTIAL_BINGMANN_SAMPLE_SORT_HEADER

/******************************************************************************/
