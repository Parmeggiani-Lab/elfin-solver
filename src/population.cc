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

/* free functions */
Candidate * new_candidate(const WorkType work_type) {
    Candidate * c = nullptr;

    switch (work_type) {
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
        ss << WorkTypeNames[work_type] << std::endl;
        throw ElfinException(ss.str());
    }

    MutationCounters dummy_mt_counters;
    c->randomize();

    return c;
}

/* protected */
/* modifiers */
void Population::release_resources() {
    delete front_buffer_;
    delete back_buffer_;
}

// static
void Population::copy_buffer(const Buffer * src, Buffer * dst) {
    dst->clear();

    const size_t src_size = src->size();
    dst->reserve(src_size);

    OMP_PAR_FOR
    for (size_t i = 0; i < src_size; i++) {
        dst->push_back(src->at(i)->clone());
    }
}

/* public */
/* ctors */
Population::Population(const WorkArea * work_area) :
    work_area_(work_area) {
    TIMING_START(init_start_time);
    {
        const size_t pop_size = OPTIONS.ga_pop_size;
        msg("Initializing population of %u...", pop_size);

        Candidate::setup(*work_area);

        front_buffer_ = new Buffer();
        front_buffer_->clear();
        front_buffer_->reserve(pop_size);

        OMP_PAR_FOR
        for (size_t i = 0; i < pop_size; i++) {
            front_buffer_->push_back(new_candidate(work_area_->type()));
        }

        // Scoring last candidate tests initialization and score func
        front_buffer_->at(pop_size - 1)->score(*work_area_);

        // Now initialize second buffer_
        swap_buffer();

        // front_buffer_ now points to nullptr
        // back_buffer_ points to the initialized const Buffer *
        // Hence we copy from back to front.
        front_buffer_ = new Buffer();
        copy_buffer(back_buffer_, front_buffer_);

        ERASE_LINE();
        msg("Initialization done\n");
    }
    TIMING_END("init", init_start_time);
}

Population::Population(const Population & other) {
    *this = other; // calls operator=(const T&)
}

Population::Population(Population && other) {
    *this = other; // calls operator=(T&&)
}

/* dtors */
Population::~Population() {
    release_resources();
}

/* modifiers */
Population & Population::operator=(const Population & other) {
    release_resources();
    front_buffer_ = new Buffer();
    copy_buffer(other.front_buffer_, front_buffer_);
    swap_buffer();
    front_buffer_ = new Buffer();
    copy_buffer(other.back_buffer_, front_buffer_);
    swap_buffer();
    return *this;
}

Population & Population::operator=(Population && other) {
    release_resources();
    front_buffer_ = other.front_buffer_;
    other.front_buffer_ = nullptr;
    back_buffer_ = other.back_buffer_;
    other.back_buffer_ = nullptr;
    work_area_ = other.work_area_;
    return *this;
}

void Population::evolve() {
    TIMING_START(evolve_start_time);
    {
        msg("Evolving population...");

        MutationCounters mc = {};

        OMP_PAR_FOR
        for (size_t i = 0; i < CUTOFFS.pop_size; i++) {
            front_buffer_->at(i)->mutate(i, mc, back_buffer_);
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

        const size_t size = front_buffer_->size();

        OMP_PAR_FOR
        for (size_t i = 0; i < size; i++) {
            front_buffer_->at(i)->score(*work_area_);
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

        std::sort(front_buffer_->begin(),
                  front_buffer_->end(),
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
        for (auto cand_ptr : *front_buffer_) {
            const Crc32 crc = cand_ptr->checksum();
            if (crc_map.find(crc) == crc_map.end()) {
                // Record a new candidate
                crc_map[crc] = cand_ptr->clone();
                unique_count++;

                if (unique_count >= CUTOFFS.survivors)
                    break;
            }
        }

        // Insert map-value-indexed individual back into population
        size_t pop_index = 0;
        for (auto & kv : crc_map) {
            delete front_buffer_->at(pop_index); // free candidate memory
            front_buffer_->at(pop_index) = kv.second;
            pop_index++;
        }

        // Sort survivors
        std::sort(front_buffer_->begin(),
                  front_buffer_->begin() + unique_count,
                  Candidate::PtrComparator);

        ERASE_LINE();
        msg("Selection done\n");
    }
    InputManager::ga_times().select_time +=
        TIMING_END("selecting", start_time_select);
}

void Population::swap_buffer() {
    const Buffer * tmp = front_buffer_;
    back_buffer_ = front_buffer_;
    front_buffer_ = const_cast<Buffer *>(tmp);
}

}  /* elfin */