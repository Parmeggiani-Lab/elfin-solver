#include "population.h"

#include <unordered_map>
#include <sstream>

#include "jutil.h"
#include "input_manager.h"
#include "mutation_counters.h"
#include "free_candidate.h"
// #include "one_hinge_candidate.h"
// #include "two_hinge_candidate.h"
#include "parallel_utils.h"

namespace elfin {

void Population::setup(const WorkArea & wa) {
    /*
     * Calculate expected length as sum of point
     * displacements over avg pair module distance
     */
    float sum_dist = 0.0f;
    const V3fList shape = wa.to_points();
    for (auto i = shape.begin() + 1; i != shape.end(); ++i)
        sum_dist += (i - 1)->dist_to(*i);

    // Add one because division counts segments. We want number of points.
    const size_t expected_len = 1 + round(sum_dist / OPTIONS.avg_pair_dist);
    Candidate::set_max_len(expected_len + OPTIONS.len_dev_alw);
}

Population::~Population() {
    for (auto & c : candidates_) {
        delete c;
    }
    candidates_.clear();
}

Candidate * Population::new_candidate() const {
    Candidate * c = nullptr;

    switch (work_area_.type()) {
    case FREE:
        c = new FreeCandidate();
        break;
    // case ONE_HINGE:
    //     c = new OneHingeCandidate();
    //     break;
    // case TWO_HINGE:
    //     c = new TwoHingeCandidate();
    //     break;
    default:
        std::stringstream ss;
        ss << "Unimplemented WorkArea type: ";
        ss << WorkTypeNames[work_area_.type()] << std::endl;
        throw ElfinException(ss.str());
    }

    MutationCounters dummy_mt_counters;
    c->mutate(-1, dummy_mt_counters, candidates_);

    return c;
}

Population::Population(const WorkArea & work_area) :
    work_area_(work_area) {
    TIMING_START(init_start_time);
    {
        const size_t size = OPTIONS.ga_pop_size;
        msg("Initializing population of %u...", size);

        candidates_.clear();
        candidates_.resize(size);

        OMP_PAR_FOR
        for (size_t i = 0; i < size; i++) {
            candidates_[i] = new_candidate();
        }

        // Scoring last candidate tests length, correct init and score func
        candidates_[size - 1]->score(work_area_);

        ERASE_LINE();
        msg("Initialization done\n");
    }
    TIMING_END("init", init_start_time);
}

Population::Population(const Population & other) :
    work_area_(other.work_area_) {
    TIMING_START(copy_start_time);
    {
        const size_t size = other.candidates_.size();
        msg("Copying population of %u...", size);

        candidates_.clear();
        candidates_.resize(size);

        OMP_PAR_FOR
        for (size_t i = 0; i < size; i++) {
            candidates_[i] = other.candidates_[i]->clone();
        }

        // Scoring last candidate tests length, correct init and score func
        candidates_[size - 1]->score(work_area_);

        ERASE_LINE();
        msg("Copying done\n");
    }
    TIMING_END("copy", copy_start_time);
}

void Population::evolve(const Population * prev_gen) {
    TIMING_START(evolve_start_time);
    {
        msg("Evolving population...");

        MutationCounters mc = {};

        OMP_PAR_FOR
        for (long i = 0; i < CUTOFFS.pop_size; i++) {
            candidates_[i]->mutate(i, mc, prev_gen->candidates());
        }

        ERASE_LINE();
        msg("Evolution done\n");

        // RNG instrumentation
        wrn(("Mutation rates:\n"
             "\tCross %.2f (fail=%.2f), Point %.2f (fail=%.2f)\n"
             "\tLimb %.2f (fail=%.2f), Rand %.2f\n"
             "\tSurvivors: %lu\n"),
            (float) mc.cross / CUTOFFS.non_survivors,
            (float) mc.cross_fail / mc.cross,
            (float) mc.point / CUTOFFS.non_survivors,
            (float) mc.point_fail / mc.point,
            (float) mc.limb / CUTOFFS.non_survivors,
            (float) mc.limb_fail / mc.limb,
            (float) mc.rand / CUTOFFS.non_survivors,
            CUTOFFS.survivors);
    }
    InputManager::ga_times().evolve_time +=
        TIMING_END("evolving", evolve_start_time);
}

void Population::score() {
    TIMING_START(score_start_time);
    {
        msg("Scoring population...");

        const size_t size = candidates_.size();

        OMP_PAR_FOR
        for (size_t i = 0; i < size; i++) {
            candidates_[i]->score(work_area_);
        }

        ERASE_LINE();
        msg("Scoring done\n");
    }
    InputManager::ga_times().score_time +=
        TIMING_END("scoring", score_start_time);
}

void Population::rank() {
    TIMING_START(rank_start_time);
    {
        msg("Ranking population...");

        std::sort(candidates_.begin(),
                  candidates_.end(),
                  Candidate::PtrComparator);

        ERASE_LINE();
        msg("Ranking done\n");
    }
    InputManager::ga_times().rank_time +=
        TIMING_END("ranking", rank_start_time);
}

void Population::select() {
    TIMING_START(start_time_select);
    {
        msg("Selecting population...");

        // Ensure variety within survivors using hashmap
        // where crc is key and cloned pointer is value.
        std::unordered_map<Crc32, Candidate *> crc_map;
        size_t unique_count = 0;

        // We don't want parallelism here because
        // the loop must priotise low indexes
        for (auto c : candidates_) {
            const Crc32 crc = c->checksum();
            if (crc_map.find(crc) == crc_map.end()) {
                // Record a new candidate
                crc_map[crc] = c->clone();
                unique_count++;

                if (unique_count >= CUTOFFS.survivors)
                    break;
            }
        }

        // Insert map-value-indexed individual back into population
        size_t pop_index = 0;
        for (auto & kv : crc_map) {
            delete candidates_[pop_index]; // free candidate memory
            candidates_[pop_index] = kv.second;
            pop_index++;
        }

        // Sort survivors
        std::sort(candidates_.begin(),
                  candidates_.begin() + unique_count,
                  Candidate::PtrComparator);

        ERASE_LINE();
        msg("Selection done\n");
    }
    InputManager::ga_times().select_time +=
        TIMING_END("selecting", start_time_select);
}

}  /* elfin */