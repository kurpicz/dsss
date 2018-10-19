/*******************************************************************************
 * src/sequential/bingmann-sample_sort.cpp
 *
 * Experiments with sequential Super Scalar String Sample-Sort (S^5).
 *
 * Binary tree search with bucket cache.
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

#include "bingmann-sample_sort.hpp"
#include "bingmann-sample_sortBSC.hpp"
#include "bingmann-sample_sortBTC.hpp"
#include "bingmann-sample_sortBTCE.hpp"
#include "bingmann-sample_sortBTCT.hpp"

#include <tlx/die.hpp>

namespace bingmann_sample_sort {

/******************************************************************************/
// Generic Variant of string sample-sort: template with a Classfier, with index
// caching, and uses in-place permutation walking for rearrangement.

template <typename Classify>
void sample_sort_generic(string* strings, size_t n, size_t depth)
{
    if (n < g_samplesort_smallsort)
    {
        g_rs_steps++;
        g_timer.change(TM_SMALLSORT);
        sample_sort_small_sort(strings, n, depth);
        g_timer.change(TM_GENERAL);
        return;
    }
    g_ss_steps++;

    // step 1: select splitters with oversampling
    g_timer.change(TM_MAKE_SAMPLE);

    const size_t numsplitters = Classify::numsplitters;
    const size_t samplesize = oversample_factor * numsplitters;

    key_type samples[samplesize];

    LCGRandom rng(&strings);

    for (size_t i = 0; i < samplesize; ++i)
        samples[i] = get_char<key_type>(strings[rng() % n], depth);

    std::sort(samples, samples + samplesize);

    g_timer.change(TM_MAKE_SPLITTER);

    Classify* classifier = new Classify;
    unsigned char* splitter_lcp = new unsigned char[numsplitters + 1];

    classifier->build(samples, samplesize, splitter_lcp);

    // step 2.2: classify all strings and count bucket sizes
    g_timer.change(TM_CLASSIFY);

    uint16_t* bktcache = new uint16_t[n];

    static const size_t bktnum = 2 * numsplitters + 1;

    classifier->classify(strings, strings + n, bktcache, depth);

    size_t* bktsize = new size_t[bktnum];
    memset(bktsize, 0, bktnum * sizeof(size_t));

    for (size_t si = 0; si < n; ++si)
        ++bktsize[bktcache[si]];

    if (debug_bucketsize)
    {
        LOG1 << "bktsize: ";
        for (size_t i = 0; i < bktnum; ++i)
        {
            LOG1 << bktsize[i];
        }
    }

    // step 3: prefix sum
    g_timer.change(TM_PREFIXSUM);

    size_t bktindex[bktnum];
    bktindex[0] = bktsize[0];
    size_t last_bkt_size = bktsize[0];
    for (size_t i = 1; i < bktnum; ++i) {
        bktindex[i] = bktindex[i - 1] + bktsize[i];
        if (bktsize[i]) last_bkt_size = bktsize[i];
    }
    assert(bktindex[bktnum - 1] == n);

    // step 4: premute in-place
    g_timer.change(TM_PERMUTE);

    for (size_t i = 0, j; i < n - last_bkt_size; )
    {
        string perm = strings[i];
        uint16_t permbkt = bktcache[i];

        while ((j = --bktindex[permbkt]) > i)
        {
            std::swap(perm, strings[j]);
            std::swap(permbkt, bktcache[j]);
        }

        strings[i] = perm;
        i += bktsize[permbkt];
    }

    delete[] bktcache;

    // step 5: recursion
    g_timer.change(TM_GENERAL);

    size_t i = 0, bsum = 0;
    while (i < bktnum - 1)
    {
        // i is even -> bkt[i] is less-than bucket
        if (bktsize[i] > 1)
        {
            LOGC(debug_recursion)
                << "Recurse[" << depth << "]: < bkt " << bsum
                << " size " << bktsize[i]
                << " lcp " << int(splitter_lcp[i / 2] & 0x7F);

            if (!g_toplevel_only)
                sample_sort_generic<Classify>(
                    strings + bsum, bktsize[i],
                    depth + (splitter_lcp[i / 2] & 0x7F));
        }
        bsum += bktsize[i++];

        // i is odd -> bkt[i] is equal bucket
        if (bktsize[i] > 1)
        {
            if (splitter_lcp[i / 2] & 0x80) {
                // equal-bucket has NULL-terminated key, done.
                LOGC(debug_recursion)
                    << "Recurse[" << depth << "]: = bkt " << bsum
                    << " size " << bktsize[i] << " is done!";
            }
            else {
                LOGC(debug_recursion)
                    << "Recurse[" << depth << "]: = bkt " << bsum
                    << " size " << bktsize[i] << " lcp keydepth!";

                if (!g_toplevel_only)
                    sample_sort_generic<Classify>(
                        strings + bsum, bktsize[i], depth + sizeof(key_type));
            }
        }
        bsum += bktsize[i++];
    }
    if (bktsize[i] > 0)
    {
        LOGC(debug_recursion)
            << "Recurse[" << depth << "]: > bkt " << bsum
            << " size " << bktsize[i] << " no lcp";

        if (!g_toplevel_only)
            sample_sort_generic<Classify>(
                strings + bsum, bktsize[i], depth);
    }
    bsum += bktsize[i++];
    assert(i == bktnum && bsum == n);

    delete[] splitter_lcp;
    delete classifier;
    delete[] bktsize;
}

/*----------------------------------------------------------------------------*/

void bingmann_sample_sortBSC(string* strings, size_t n)
{
    using Classify = ClassifyBinarySearch<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

/*----------------------------------------------------------------------------*/

void bingmann_sample_sortBTC(string* strings, size_t n)
{
    using Classify = ClassifyTreeSimple<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCA(string* strings, size_t n)
{
    using Classify = ClassifyTreeAssembler<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCU(string* strings, size_t n)
{
    using Classify = ClassifyTreeUnroll<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCUI(string* strings, size_t n)
{
    using Classify = ClassifyTreeUnrollInterleave<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

/*----------------------------------------------------------------------------*/

void bingmann_sample_sortBTCT(string* strings, size_t n)
{
    using Classify = ClassifyTreeCalcSimple<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCTU(string* strings, size_t n)
{
    using Classify = ClassifyTreeCalcUnroll<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCTUI(string* strings, size_t n)
{
    using Classify = ClassifyTreeCalcUnrollInterleave<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

/*----------------------------------------------------------------------------*/

void bingmann_sample_sortBTCE(string* strings, size_t n)
{
    using Classify = ClassifyEqual<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCEA(string* strings, size_t n)
{
    using Classify = ClassifyEqualAssembler<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCEU(string* strings, size_t n)
{
    using Classify = ClassifyEqualUnrollAssembler<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

void bingmann_sample_sortBTCEV(string* strings, size_t n)
{
    using Classify = ClassifyEqualUnroll<>;
    sample_sort_pre();
    g_stats >> "numsplitters" << size_t(Classify::numsplitters)
        >> "splitter_treebits" << size_t(Classify::treebits);
    sample_sort_generic<Classify>(strings, n, 0);
    sample_sort_post();
}

} // namespace bingmann_sample_sort

/******************************************************************************/
