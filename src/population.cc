#include "population.h"

#include <unordered_map>
#include <sstream>

#include "input_manager.h"
#include "mutation_modes.h"
#include "parallel_utils.h"
#include "basic_node_team.h"

namespace elfin {

/* free */
NodeTeamSP create_team(WorkArea const* wa) {
    switch (wa->type()) {
    case WorkType::FREE:
        return std::make_shared<BasicNodeTeam>(wa);
    // case WorkType::ONE_HINGE:
    //     return std::make_shared<OneHingeNodeTeam>(wa);
    // case WorkType::TWO_HINGE:
    //     return std::make_shared<TwoHingeNodeTeam>(wa);
    default:
        die("Unimplemented work type: %s\n",
            WorkTypeToCStr(wa->type()));
        return nullptr; // Suppress no return warning
    }
}

/* public */
/* ctors */
Population::Population(WorkArea const* work_area) {
    TIMING_START(init_start_time);
    {
        size_t const pop_size = OPTIONS.ga_pop_size;
        msg("Initializing population of %u...", pop_size);

        NodeTeams* new_front_buffer = &teams[0];
        NodeTeams* new_back_buffer = &teams[1];

        // Must pre allocate for parallel assignment
        new_front_buffer->resize(pop_size);
        new_back_buffer->resize(pop_size);

        OMP_PAR_FOR
        for (size_t i = 0; i < pop_size; i++) {
            new_front_buffer->at(i) = create_team(work_area);
            new_front_buffer->at(i)->randomize();
            new_back_buffer->at(i) = create_team(work_area);
        }

        front_buffer_ = new_front_buffer;
        back_buffer_ = new_back_buffer;

        ERASE_LINE();
        msg("Initialization done\n");
    }
    TIMING_END("init", init_start_time);
}

/* modifiers */
void Population::evolve() {
    TIMING_START(evolve_start_time);
    {
        msg("Evolving population...");

        MutationMode mutation_mode_tally[CUTOFFS.pop_size] = {};

        OMP_PAR_FOR
        for (size_t rank = 0; rank < CUTOFFS.pop_size; rank++) {
            MutationMode mode = MutationMode::NONE;

            auto& team = front_buffer_->at(rank);
            // Rank is 0-indexed, hence <
            if (rank < CUTOFFS.survivors) {
                *team = *(back_buffer_->at(rank));
                mode = MutationMode::NONE;
            }
            else {
                // Replicate mother
                size_t const mother_id =
                    random::get_dice(CUTOFFS.survivors); // only elites
                auto& mother_team = back_buffer_->at(mother_id);

                size_t const father_id =
                    random::get_dice(CUTOFFS.pop_size); // include all back_buffer_ teams
                auto& father_team = back_buffer_->at(father_id);

                mode = team->mutate_and_score(*mother_team, *father_team);
            }

            mutation_mode_tally[rank] = mode;
        }

        MutationCounter mc;
        for (MutationMode const& mode : mutation_mode_tally) {
            mc[mode]++;
        }

        ERASE_LINE();
        msg("Evolution done\n");

        // RNG instrumentation
        MutationModeList mutation_modes = gen_mutation_mode_list();
        std::ostringstream mutation_ss;
        mutation_ss << "Mutation Ratios:\n";
        for (MutationMode mode : mutation_modes) {
            mutation_ss << "    " << MutationModeToCStr(mode) << ':';

            float const mode_ratio = 100.f * mc[mode] / CUTOFFS.non_survivors;
            mutation_ss << " " << string_format("%.1f", mode_ratio) << "% ";
            mutation_ss << "(" << mc[mode] << "/" << CUTOFFS.non_survivors << ")\n";
        }

        msg("%s\n", mutation_ss.str().c_str());
    }
    InputManager::ga_times().evolve_time +=
        TIMING_END("evolving", evolve_start_time);
}

void Population::rank() {
    TIMING_START(rank_start_time);
    {
        msg("Ranking population...");

        std::sort(begin(*front_buffer_),
                  end(*front_buffer_),
                  NodeTeam::ScoreCompareSP);

        ERASE_LINE();
        msg("Ranking done\n");
    }
    InputManager::ga_times().rank_time +=
        TIMING_END("ranking", rank_start_time);
}

void Population::select() {
    /*
     * Minimize same-ness within survivors by using hashmap where crc is key
     * and cloned pointer is value.
     */

    TIMING_START(start_time_select);
    {
        msg("Selecting population...");

        std::unordered_map<Crc32, NodeTeamSP> crc_map;
        size_t unique_count = 0;

        // We don't want parallelism here because the low indexes must be
        // prioritized.
        for (auto& team : *front_buffer_) {
            Crc32 const crc = team->checksum();
            if (crc_map.find(crc) == end(crc_map)) {
                // Record a new team
                crc_map[crc] = team->clone();
                unique_count++;

                if (unique_count >= CUTOFFS.survivors) {
                    break;
                }
            }
        }

        // Insert map-value-indexed individual back into population
        size_t pop_index = 0;
        for (auto& kv : crc_map) {
            front_buffer_->at(pop_index) = std::move(kv.second);
            pop_index++;
        }

        // Sort survivors
        std::sort(begin(*front_buffer_),
                  begin(*front_buffer_) + unique_count,
                  NodeTeam::ScoreCompareSP);

        ERASE_LINE();
        msg("Selection done\n");
    }
    InputManager::ga_times().select_time +=
        TIMING_END("selecting", start_time_select);
}

void Population::swap_buffer() {
    NodeTeams const* tmp = back_buffer_;
    back_buffer_ = front_buffer_;
    front_buffer_ = const_cast<NodeTeams *>(tmp);
}

}  /* elfin */