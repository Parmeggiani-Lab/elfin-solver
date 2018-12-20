#include "output_manager.h"

#include <sstream>

#include "json.h"
#include <jutil/jutil.h>

namespace elfin {

//
// Resolves file name from path string.
// https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
// (wild)
//
std::string get_filename(const std::string& path) {
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


void OutputManager::write_output(
    EvolutionSolver const& solver,
    std::string extra_dir,
    size_t const indent_size) {
    if (not solver.has_result()) {
        wrn("Solver %p has no result to be written out\n", &solver);
        return;
    }

    // Compute final output dir string.
    std::ostringstream output_dir_ss;
    output_dir_ss << OPTIONS.output_dir << "/"
                  << extra_dir;
    std::string output_dir_str = output_dir_ss.str();
    mkdir_ifn_exists(output_dir_str.c_str());

    JSON data;
    for (auto& itr : SPEC.work_areas()) {
        std::string const& wa_name = itr.first;
        auto& wa = itr.second;
        JSON waj;

        try {
            std::vector<NodeTeamSP> const& solutions =
                solver.best_sols().at(wa_name);
            for (size_t i = 0; i < solutions.size(); ++i)
            {
                JSON sol_json;
                auto& team = solutions.at(i);

                if (team) {
                    auto node_names = team->get_node_names();
                    sol_json["nodes"] = node_names;
                    wrn("Output format not complete\n");
                    //
                    // TODO(@joy13975):
                    // 1. Nodes need to express 4x4 tx in result.
                    // 2. Nodes need to be transformed to solution frame.
                    //
                    sol_json["score"] = team->score();
                }
                else {
                    err("Null candidate in work area \"%s\"\n", wa_name.c_str());
                }

                waj[i] = sol_json;
            }
            data[wa_name] = waj;
        }
        catch (std::out_of_range& e)
        {
            err("WorkArea \"%s\" has no solutions\n", wa_name.c_str());
        }
    }

    // Generate JSON output path.
    std::ostringstream json_output_path_ss;
    json_output_path_ss << output_dir_str << "/"
                        << get_filename(OPTIONS.input_file)
                        << ".json";

    std::string json_out_path_str = json_output_path_ss.str();
    char const* json_output_path = json_out_path_str.c_str();

    // At last, write JSON.
    std::string dump = data.dump(indent_size);
    write_binary(json_output_path,
                 dump.c_str(),
                 dump.size());
}

}  /* elfin */