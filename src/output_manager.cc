#include "output_manager.h"

#include <sstream>

#include "json.h"
#include "jutil.h"

namespace elfin {

/*
 * Resolves file name from path string.
 * https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
 * (wild)
 */
std::string get_filename(const std::string& path) {
    std::string filename = path;
    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        filename.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    const size_t period_idx = filename.rfind('.');
    if (std::string::npos != period_idx) {
        filename.erase(period_idx);
    }
    return filename;
}

// static
void OutputManager::write_output(
    const EvolutionSolver* solver,
    std::string extra_dir,
    const size_t indent_size) {
    // Compute final output dir string
    std::ostringstream output_dir_ss;
    output_dir_ss << OPTIONS.output_dir << "/"
                  << extra_dir;
    std::string output_dir_str = output_dir_ss.str();
    mkdir_ifn_exists(output_dir_str.c_str());

    JSON data;
    for (auto& kv : SPEC.work_area_map()) {
        const std::string& wa_name = kv.first;
        const WorkArea& wa = kv.second;
        JSON waj;

        try {
            const std::vector<std::shared_ptr<Candidate>> & candidates =
                        solver->best_sols().at(wa_name);
            for (size_t i = 0; i < candidates.size(); ++i)
            {
                JSON cand_json;
                const std::shared_ptr<Candidate> cand_ptr = candidates.at(i);

                if (cand_ptr) {
                    auto node_names = cand_ptr->get_node_names();
                    cand_json["nodes"] = node_names;
                    wrn("Output format not complete\n");
                    /*
                    TODO:
                    1. Nodes need to express 4x4 tx in result
                    2. Nodes need to be transformed to solution frame
                    */
                    cand_json["score"] = cand_ptr->score();
                }
                else {
                    err("Null candidate in work area \"%s\"\n", wa_name.c_str());
                }

                waj[i] = cand_json;
            }
            data[wa_name] = waj;
        }
        catch (std::out_of_range& e)
        {
            err("WorkArea \"%s\" has no solutions\n", wa_name.c_str());
        }
    }

    // Generate JSON output path
    std::ostringstream json_output_path_ss;
    json_output_path_ss << output_dir_str << "/"
                        << get_filename(OPTIONS.input_file)
                        << ".json";

    std::string json_out_path_str = json_output_path_ss.str();
    const char* json_output_path = json_out_path_str.c_str();

    // At last, write JSON
    std::string dump = data.dump(indent_size);
    write_binary(json_output_path,
                 dump.c_str(),
                 dump.size());
}

}  /* elfin */