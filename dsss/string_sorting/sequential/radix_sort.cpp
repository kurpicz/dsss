/*******************************************************************************
 * string_sorting/sequential/radix_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "rantala/msd_a.hpp"
#include "rantala/msd_a2.hpp"
#include "rantala/msd_ce.hpp"
#include "rantala/msd_ci.hpp"
#include "rantala/msd_dyn_block.hpp"
#include "rantala/msd_dyn_vector.hpp"
#include "rantala/msd_lsd.hpp"
#include "sequential/bingmann-radix_sort.hpp"
#include "string_sorting/util/algorithm.hpp"

// Includes for rantala/msd_dyn_vector.hpp
#include <vector>
#include "rantala/tools/vector_malloc.hpp"
#include "rantala/tools/vector_realloc.hpp"
#include "rantala/tools/vector_block.hpp"
#include "rantala/tools/vector_bagwell.hpp"
#include "rantala/tools/vector_brodnik.hpp"

namespace bingmann {

REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE0,
  "sequential/msd_CE0",
  "sequential/msd_CE0 (rantala CE0 baseline)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE0_generic,
  "sequential/msd_CE0_gen",
  "sequential/msd_CE0 generic (rantala CE baseline)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE1,
  "sequential/msd_CE1",
  "sequential/msd_CE1 (with charcache, fused loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE2,
  "sequential/msd_CE2",
  "sequential/msd_CE2 (with charcache, fissioned loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE3,
  "sequential/msd_CE3",
  "sequential/msd_CE3 (with charcache, fissioned loop, 16-bit adaptive)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI0,
  "sequential/msd_CI0",
  "sequential/msd_CI0 (in-place baseline)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI1,
  "sequential/msd_CI1",
  "sequential/msd_CI1 (with charcache, fused loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI2,
  "sequential/msd_CI2",
  "sequential/msd_CI2 (with charcache, fissioned loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI2_generic,
  "sequential/msd_CI2_gen",
  "sequential/msd_CI2 generic (with charcache, fissioned loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI3,
  "sequential/msd_CI3",
  "sequential/msd_CI3 (with charcache, fissioned loop, 16-bit adaptive)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE0_sb,
  "sequential/msd_CE0_sb",
  "sequential/msd_CE0_sb (CE0 stack-based)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE2_sb,
  "sequential/msd_CE2_sb",
  "sequential/msd_CE2_sb (CE2 stack-based, charcache, fissioned loop)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CE3_sb,
  "sequential/msd_CE3_sb",
  "sequential/msd_CE3_sb (CE3 stack-based, charcache, fissioned loop, "
    "16-bit adaptive)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI2_sb,
  "sequential/msd_CI2_sb",
  "sequential/msd_CI2_sb (CI stack-based, charcache, fissioned)")
REGISTER_SEQUENTIAL_SORTER(bingmann_msd_CI3_sb,
  "sequential/msd_CI3_sb",
  "sequential/msd_CI3_sb (CI stack-based, charcache, fissioned, "
    "16-bit adaptive)")

} // namespace bingmann

namespace rantala_msd_a {

REGISTER_SEQUENTIAL_SORTER(msd_A,
  "rantala/msd_A",
  "msd_A")
REGISTER_SEQUENTIAL_SORTER(msd_A_adaptive,
  "rantala/msd_A_adaptive",
  "msd_A_adaptive")

} //rantala_msd_a

namespace rantala_msd_a2 {

REGISTER_SEQUENTIAL_SORTER(msd_A2,
  "rantala/msd_A2",
  "msd_A2")
REGISTER_SEQUENTIAL_SORTER(msd_A2_adaptive,
  "rantala/msd_A2_adaptive",
  "msd_A2_adaptive")

} // namespace rantala_msd_a2

namespace rantala {

REGISTER_SEQUENTIAL_SORTER(msd_CE0,
  "rantala/msd_CE0",
  "msd_CE0: baseline")
REGISTER_SEQUENTIAL_SORTER(msd_CE1,
  "rantala/msd_CE1",
  "msd_CE1: oracle")
REGISTER_SEQUENTIAL_SORTER(msd_CE2,
  "rantala/msd_CE2",
  "msd_CE2: oracle+loop fission")
REGISTER_SEQUENTIAL_SORTER(msd_CE3,
  "rantala/msd_CE3",
  "msd_CE3: oracle+loop fission+adaptive")
REGISTER_SEQUENTIAL_SORTER(msd_CE4,
  "rantala/msd_CE4",
  "msd_CE4: oracle+loop fission+adaptive+16bit counter")
REGISTER_SEQUENTIAL_SORTER(msd_CE5,
  "rantala/msd_CE5",
  "msd_CE5: oracle+loop fission+adaptive+16bit counter+prealloc")
REGISTER_SEQUENTIAL_SORTER(msd_CE6,
  "rantala/msd_CE6",
  "msd_CE6: oracle+loop fission+adaptive+16bit counter+prealloc+unroll")
REGISTER_SEQUENTIAL_SORTER(msd_CE7,
  "rantala/msd_CE7",
  "msd_CE7: oracle+loop fission+adaptive+16bit counter+prealloc+unroll+sortedness")

REGISTER_SEQUENTIAL_SORTER(msd_ci,
  "rantala/msd_CI",
  "msd_CI")

REGISTER_SEQUENTIAL_SORTER(msd_ci_adaptive,
  "rantala/msd_CI_adaptive",
  "msd_CI: adaptive")

REGISTER_SEQUENTIAL_SORTER(msd_A_lsd4,
  "rantala/msd_A_lsd4",
  "msd_A_lsd with 4byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd6,
  "rantala/msd_A_lsd6",
  "msd_A_lsd with 6byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd8,
  "rantala/msd_A_lsd8",
  "msd_A_lsd with 8byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd10,
  "rantala/msd_A_lsd10",
  "msd_A_lsd with 10byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd12,
  "rantala/msd_A_lsd12",
  "msd_A_lsd with 12byte cache")

REGISTER_SEQUENTIAL_SORTER(msd_A_lsd_adaptive4,
  "rantala/msd_A_lsd_adaptive4",
  "msd_A_lsd_adaptive with 4byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd_adaptive6,
  "rantala/msd_A_lsd_adaptive6",
  "msd_A_lsd_adaptive with 6byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd_adaptive8,
  "rantala/msd_A_lsd_adaptive8",
  "msd_A_lsd_adaptive with 8byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd_adaptive10,
  "rantala/msd_A_lsd_adaptive10",
  "msd_A_lsd_adaptive with 10byte cache")
REGISTER_SEQUENTIAL_SORTER(msd_A_lsd_adaptive12,
  "rantala/msd_A_lsd_adaptive12",
  "msd_A_lsd_adaptive with 12byte cache")


} // namespace rantala

namespace rantala_msd_db {

REGISTER_SEQUENTIAL_SORTER(msd_DB,
  "rantala/msd_DB",
  "msd_DB")

} // namespace rantala_msd_db

namespace rantala_msd_dv {

#define MAKE_ALG2(name, vec)                                                   \
void msd_D_##name(unsigned char** strings, size_t n)                           \
{                                                                              \
        vec<unsigned char*> buckets[256];                                      \
        msd_D<vec<unsigned char*>, size_t>(strings, n, 0, buckets);            \
}                                                                              \
REGISTER_SEQUENTIAL_SORTER(msd_D_##name, "rantala/msd_D_"#name,                \
              "msd_D_"#name)                                                   \
void msd_D_##name##_adaptive(unsigned char** strings, size_t n)                \
{                                                                              \
        vec<unsigned char*>* buckets = new vec<unsigned char*>[0x10000];       \
        msd_D_adaptive(strings, n, 0, buckets);                                \
        delete [] buckets;                                                     \
}                                                                              \
REGISTER_SEQUENTIAL_SORTER(msd_D_##name##_adaptive,                            \
  "rantala/msd_D_"#name"_adaptive", "msd_D_"#name"_adaptive")

#define MAKE_ALG1(vec) MAKE_ALG2(vec, vec)

MAKE_ALG2(std_vector, std::vector)
MAKE_ALG2(std_deque, std::deque)
MAKE_ALG2(std_list, counting_list)
MAKE_ALG1(vector_realloc)
MAKE_ALG1(vector_malloc)
MAKE_ALG1(vector_realloc_counter_clear)
MAKE_ALG1(vector_malloc_counter_clear)
MAKE_ALG1(vector_realloc_shrink_clear)
MAKE_ALG1(vector_block)
MAKE_ALG1(vector_brodnik)
MAKE_ALG1(vector_bagwell)

#undef MAKE_ALG1
#undef MAKE_ALG2

} // namespace rantala_msd_dv

/******************************************************************************/
