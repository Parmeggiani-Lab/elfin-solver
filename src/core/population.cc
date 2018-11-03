#include "population.h"

#include <unordered_map>

#include "free_candidate.h"
#include "parallel_utils.h"

namespace elfin {

Population::Population(const Options & options, const WorkArea & work_area) :
    work_area_(work_area) {

    counts_.pop_size = options.ga_pop_size;

    counts_.survivors = std::round(options.ga_survive_rate * options.ga_pop_size);

    counts_.non_survivors = (options.ga_pop_size - counts_.survivors);

    mutation_cutoffs_.cross =
        counts_.survivors +
        std::round(options.ga_cross_rate * counts_.non_survivors);

    mutation_cutoffs_.point =
        mutation_cutoffs_.cross +
        std::round(options.ga_point_mutate_rate * counts_.non_survivors);

    mutation_cutoffs_.limb =
        std::min(
            (ulong) (mutation_cutoffs_.point +
                     std::round(options.ga_limb_mutate_rate * counts_.non_survivors)),
            counts_.pop_size);

    /*
     * Calculate expected length as sum of point
     * displacements over avg pair module distance
     */
    float sum_dist = 0.0f;
    const Points3f shape = work_area.to_points3f(); 
    for (auto i = shape.begin() + 1; i != shape.end(); ++i)
        sum_dist += (i - 1)->dist_to(*i);

    // Add one because division counts segments. We want number of points.
    int expected_len = 1 + round(sum_dist / options.avg_pair_dist);
    counts_.len.expected = expected_len;
    counts_.len.min = std::max((int) (expected_len - options.len_dev_alw), 1);
    counts_.len.max = expected_len + options.len_dev_alw;
};

void Population::init(size_t size, bool randomize) {
    TIMING_START(init_start_time);
    {
        candidates_.clear();
        candidates_.resize(size);

        const size_t count_block = size / 10;

        OMP_PAR_FOR
        for (size_t i = 0; i < size; i++)
        {
#ifndef _TARGET_GPU
            if (i % count_block == 0)
            {
                if (i > 0) {
                    ERASE_LINE();
                }
                msg("Initialising population: %.2f%%", (float) i / size);
            }
#endif

            candidates_[i] = work_area_.new_candidate(randomize);
        }

        // Scoring last candidate tests length, correct init and score func
        candidates_[size - 1]->score(work_area_);
        ERASE_LINE();
        msg("Initialising population: 100%%");
    }
    TIMING_END("init", init_start_time);
}

void Population::evolve(const Population * prev_gen) {
    TIMING_START(evolve_start_time);
    {
        // Probabilistic evolution
        msg("Evolution: %.2f%%", (float) 0.0f);

        MutationCounters mt_counts;

        const ulong ga_pop_block = counts_.pop_size / 10;

        OMP_PAR_FOR
        for (int i = counts_.survivors; i < counts_.pop_size; i++)
        {
            candidates_[i]->mutate(i, mutation_cutoffs_, mt_counts);

#ifndef _TARGET_GPU
            if (i % ga_pop_block == 0)
            {
                ERASE_LINE();
                msg("Evolution: %.2f%%", (float) i / counts_.pop_size);
            }
#endif
        }

        ERASE_LINE();
        msg("Evolution: 100%% Done\n");

        // Keep some actual counts to make sure the RNG is working
        // correctly
        dbg("Mutation rates: cross %.2f (fail=%d), pm %.2f, lm %.2f, rand %.2f, survivalCount: %d\n",
            (float) mt_counts.cross / counts_.non_survivors,
            mt_counts.cross_fail,
            (float) mt_counts.point / counts_.non_survivors,
            (float) mt_counts.limb / counts_.non_survivors,
            (float) mt_counts.rand / counts_.non_survivors,
            counts_.survivors);
    }
    counts_.evolve_time += TIMING_END("evolving", evolve_start_time);
}

void Population::score() {
    TIMING_START(score_start_time);
    {
        const size_t size = candidates_.size();
        const ulong count_block = size / 10;

        OMP_PAR_FOR
        for (int i = 0; i < size; i++)
        {
#ifndef _TARGET_GPU
            if (i % count_block == 0)
            {
                if (i > 0) {
                    ERASE_LINE();
                }
                msg("Scoring: %.2f%%",  (float) i / size);
            }
#endif
            candidates_[i]->score(work_area_);
        }

        ERASE_LINE();
        msg("Scoring: 100%%\n");
    }
    counts_.score_time += TIMING_END("scoring", score_start_time);
}

void Population::rank() {
    TIMING_START(rank_start_time);
    {
        std::sort(candidates_.begin(),
                  candidates_.end());
    }
    counts_.rank_time += TIMING_END("ranking", rank_start_time);
}

void Population::select() {
    TIMING_START(start_time_select);
    {
        // Select distinct survivors

        // Ensure variety within survivors using hashmap
        // and crc as key
        std::unordered_map<Crc32, Candidate *> crc_map;
        ulong unique_count = 0;

        // We don't want parallelism here because
        // the loop must priotise low indexes
        for (auto & c : candidates_)
        {
            const Crc32 crc = c->checksum();
            if (crc_map.find(crc) == crc_map.end())
            {
                // This individual is a new one - record
                crc_map[crc] = c;
                unique_count++;

                if (unique_count >= counts_.survivors)
                    break;
            }
        }

        // Insert map-value-indexed individual back into population
        ulong popIndex = 0;
        for (auto & kv : crc_map) {
            candidates_[popIndex++] = kv.second;
        }

        // Sort survivors
        std::sort(candidates_.begin(),
                  candidates_.begin() + unique_count);
    }
    counts_.select_time += TIMING_END("selecting", start_time_select);
}

}  /* elfin */