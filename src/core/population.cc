#include "population.h"

#include <unordered_map>

#include "free_candidate.h"
#include "parallel_utils.h"

namespace elfin {

Population::Population(const Options & options, const WorkArea & work_area) :
    work_area_(work_area) {

    surviver_cutoff_ = std::round(options.ga_survive_rate * options.ga_pop_size);

    non_surviver_count_ = (options.ga_pop_size - surviver_cutoff_);
};

void Population::init(size_t size, bool randomize) {
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

void Population::evolve(const Population * prev_gen) {
}

void Population::score() {
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

void Population::rank() {
    std::sort(candidates_.begin(),
              candidates_.end());
}

void Population::select() {
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

            if (unique_count >= surviver_cutoff_)
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

}  /* elfin */