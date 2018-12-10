#include "population.h"

#include <unordered_map>
#include <sstream>

#include "jutil.h"
#include "input_manager.h"
#include "mutation_modes.h"
#include "basic_node_team.h"
#include "parallel_utils.h"

namespace elfin {

/* free functions */
Candidate * new_candidate(const WorkType work_type) {
    NodeTeam * node_team = nullptr;

    switch (work_type) {
    case WorkType::FREE:
        node_team = new BasicNodeTeam();
        break;
    // case WorkType::ONE_HINGE:
    //     node_team = new OneHingeNodeTeam();
    //     break;
    // case WorkType::TWO_HINGE:
    //     node_team = new TwoHingeNodeTeam();
    //     break;
    default:
        std::ostringstream ss;
        ss << "Unimplemented WorkArea type: ";
        ss << WorkTypeToCStr(work_type) << std::endl;
        throw ElfinException(ss.str());
    }

    Candidate * candidate = new Candidate(node_team);
    candidate->randomize();

    return candidate;
}

/* protected */
/* modifiers */
void Population::release_resources() {
    delete front_buffer_;
    delete back_buffer_;
}

// static
void Population::copy_buffer(const Buffer * src, Buffer * dst) {
    NICE_PANIC(src == dst);

    dst->clear();

    const size_t src_size = src->size();
    dst->resize(src_size);

    OMP_PAR_FOR
    for (size_t i = 0; i < src_size; i++) {
        dst->at(i) = src->at(i)->clone();
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
        front_buffer_->resize(pop_size); // Must pre allocate for parallel assignment

        OMP_PAR_FOR
        for (size_t i = 0; i < pop_size; i++) {
            front_buffer_->at(i) = new_candidate(work_area_->type());
        }

        // Scoring last candidate tests initialization and score func
        front_buffer_->at(pop_size - 1)->calc_score(work_area_);

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
    copy_buffer(other.back_buffer_, front_buffer_);
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

        MutationMode mutation_mode_tally[CUTOFFS.pop_size] = {};

        OMP_PAR_FOR
        for (size_t i = 0; i < CUTOFFS.pop_size; i++) {
            mutation_mode_tally[i] = front_buffer_->at(i)->mutate(
                                    i,
                                    back_buffer_);
        }

        MutationCounter mc;
        for(const MutationMode & mode : mutation_mode_tally) {
            mc[mode]++;
        }

        ERASE_LINE();
        msg("Evolution done\n");

        // RNG instrumentation
        MutationModeList mutation_modes = gen_mutation_mode_list();
        std::ostringstream mutation_ss;
        mutation_ss << "Mutation Ratios (out of " << CUTOFFS.survivors << " non-survivors):\n";
        for (MutationMode mode : mutation_modes) {
            mutation_ss << "    " << MutationModeToCStr(mode) << ':';

            const float mode_ratio = 100.f * mc[mode] / CUTOFFS.non_survivors;
            mutation_ss << " " << string_format("%.1f", mode_ratio) << "% ";
            mutation_ss << "(" << mc[mode] << "/" << CUTOFFS.non_survivors << ")\n";
        }

        msg("%s\n", mutation_ss.str().c_str());
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
            front_buffer_->at(i)->calc_score(work_area_);
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

        // Ensure variety within survivors using hashmap where crc is key and
        // cloned pointer is value.
        std::unordered_map<Crc32, Candidate *> crc_map;
        size_t unique_count = 0;

        // We don't want parallelism here because the loop must prioritise low
        // indexes
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
    const Buffer * tmp = back_buffer_;
    back_buffer_ = front_buffer_;
    front_buffer_ = const_cast<Buffer *>(tmp);
}

}  /* elfin */