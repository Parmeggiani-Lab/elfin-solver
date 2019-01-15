#include "output_manager.h"

#include <sstream>

#include "json.h"
#include "jutil.h"

namespace elfin {

//
// Resolves file name from path string.
// https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
// (free)
//
std::string get_filename(std::string const& path) {
    std::string filename = path;
    // Remove directory if present.

    // Do this before extension removal in case directory has a period character.
    size_t const last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        filename.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    size_t const period_idx = filename.rfind('.');
    if (std::string::npos != period_idx) {
        filename.erase(period_idx);
    }
    return filename;
}


void OutputManager::write_output(EvolutionSolver const& solver,
                                 std::string extra_dir,
                                 size_t const indent_size) {
    if (not solver.has_result()) {
        JUtil.warn("Solver %p has no result to be written out\n", &solver);
        return;
    }

    JUtil.info("Writing results\n");

    // Compute final output dir string.
    std::ostringstream output_dir_ss;
    output_dir_ss << OPTIONS.output_dir << "/"
                  << extra_dir;
    std::string output_dir_str = output_dir_ss.str();
    JUtil.mkdir_ifn_exists(output_dir_str.c_str());

    try {
        JSON output_json;
        for (auto& [wa_name, wa] : SPEC.work_areas()) {
            JSON work_area_json;

            try {
                auto solutions = solver.best_sols(wa_name);  // TeamPtrMaxHeap

                size_t i = 0;
                while (not solutions.empty()) {
                    auto team = solutions.top();
                    solutions.pop();
                    if (team) {
                        JSON sol_json;
                        sol_json["nodes"] = team->to_json();
                        sol_json["score"] = team->score();
                        work_area_json[i++] = sol_json;
                    }
                    else {
                        JUtil.error("team=%p in %s\n", team, wa_name);
                    }
                }
                output_json[wa_name] = work_area_json;
            }
            catch (std::out_of_range& e)
            {
                JUtil.error("WorkArea \"%s\" has no solutions\n", wa_name.c_str());
            }
        }

        // Generate JSON output path.
        std::ostringstream json_output_path_ss;
        json_output_path_ss << output_dir_str << "/"
                            << get_filename(OPTIONS.spec_file)
                            << ".json";

        std::string json_out_path_str = json_output_path_ss.str();
        char const* json_output_path = json_out_path_str.c_str();

        // At last, write JSON to file.
        std::string dump = output_json.dump(indent_size);
        JUtil.write_binary(json_output_path,
                           dump.c_str(),
                           dump.size());
    } catch (JSON::exception const& je) {
        JSON_LOG_EXIT(je);
    }
}

}  /* elfin */