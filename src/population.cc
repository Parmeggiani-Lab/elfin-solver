#include "population.h"

#include <unordered_map>
#include <sstream>

#include "input_manager.h"
#include "parallel_utils.h"
#include "path_team.h"

namespace elfin {

void print_mutation_ratios(mutation::ModeList mode_tally) {
    if (JUtil.check_log_lvl(LOGLVL_DEBUG)) {
        mutation::Counter mc;
        for (mutation::Mode const& mode : mode_tally) {
            mc[mode]++;
        }

        auto mutation_modes = mutation::gen_mode_list();
        std::ostringstream mutation_ss;
        mutation_ss << "Mutation Ratios (out of " << CUTOFFS.non_survivors << "):\n";
        for (mutation::Mode mode : mutation_modes) {
            mutation_ss << "  " << mutation::ModeToCStr(mode) << ':';

            float const mode_ratio = 100.f * mc[mode] / CUTOFFS.non_survivors;
            mutation_ss << " " << string_format("%.1f", mode_ratio) << "% ";
            mutation_ss << "(" << mc[mode] << ")\n";
        }

        JUtil.debug("%s\n", mutation_ss.str().c_str());
    }
}

/* public */
/* ctors */
Population::Population(WorkArea const* work_area) {
    TIMING_START(init_start_time);
    {
        size_t const pop_size = OPTIONS.ga_pop_size;
        JUtil.info("Initializing population of %u...\n", pop_size);

        NodeTeams* new_front_buffer = &teams[0];
        NodeTeams* new_back_buffer = &teams[1];

        // Must pre allocate for parallel assignment
        new_front_buffer->resize(pop_size);
        new_back_buffer->resize(pop_size);

        OMP_PAR_FOR
        for (size_t i = 0; i < pop_size; i++) {
            new_back_buffer->at(i) = NodeTeam::create_team(work_area);
            new_back_buffer->at(i)->randomize();
            new_front_buffer->at(i) = NodeTeam::create_team(work_area);
        }

        front_buffer_ = new_front_buffer;
        back_buffer_ = new_back_buffer;
    }
    TIMING_END("initialization", init_start_time);
}

/* modifiers */
void Population::evolve() {
    TIMING_START(evolve_start_time);
    {
        JUtil.info("Evolving population...\n");

        mutation::ModeList mutation_mode_tally;
        mutation_mode_tally.reserve(CUTOFFS.pop_size);

        OMP_PAR_FOR
        for (size_t rank = 0; rank < CUTOFFS.pop_size; rank++) {
            mutation::Mode mode = mutation::Mode::NONE;

            auto& team = front_buffer_->at(rank);
            // Rank is 0-indexed, hence <
            if (rank < CUTOFFS.survivors) {
                team->copy(*back_buffer_->at(rank));
                mode = mutation::Mode::NONE;
            }
            else {
                // Replicate mother
                size_t const mother_id =
                    random::get_dice(CUTOFFS.survivors); // only elites
                auto& mother_team = back_buffer_->at(mother_id);

                size_t const father_id =
                    random::get_dice(CUTOFFS.pop_size); // include all back_buffer_ teams
                auto& father_team = back_buffer_->at(father_id);

                mode = team->evolve(*mother_team, *father_team);
            }

            mutation_mode_tally[rank] = mode;
        }

        print_mutation_ratios(mutation_mode_tally);
    }
    InputManager::ga_times().evolve_time +=
        TIMING_END("evolution", evolve_start_time);
}

void Population::rank() {
    TIMING_START(rank_start_time);
    {
        JUtil.info("Ranking population...\n");

        std::sort(begin(*front_buffer_),
                  end(*front_buffer_),
                  NodeTeam::ScoreCompareSP);
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
        JUtil.info("Selecting population...\n");

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
    }
    InputManager::ga_times().select_time +=
        TIMING_END("variety selection", start_time_select);
}

void Population::swap_buffer() {
    NodeTeams const* tmp = back_buffer_;
    back_buffer_ = front_buffer_;
    front_buffer_ = const_cast<NodeTeams *>(tmp);
}

}  /* elfin */