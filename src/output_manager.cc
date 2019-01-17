#include "output_manager.h"

#include <sstream>

#include "spec.h"
#include "json.h"
#include "jutil.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct OutputManager::PImpl : public PImplBase<OutputManager> {
    using PImplBase::PImplBase;

    /* data */
    JSON output_json;

    /* accessors */
    // Resolves file name from path string.
    // https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
    static std::string get_filename(std::string const& path) {
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

    void write_to_file(Options const& options,
                       size_t const indent_size) const {
        JUtil.info("Writing results...\n");

        JUtil.mkdir_ifn_exists(options.output_dir.c_str());

        // Build JSON output path.
        std::string const& output_path =
            options.output_dir + "/" + get_filename(options.spec_file) + ".json";

        std::string const& dump = output_json.dump(indent_size);
        JUtil.write_binary(output_path.c_str(),
                           dump.c_str(),
                           dump.size());
    }

    /* modifiers */
    void collect_output(Spec const & spec) {
        for (auto const& [wa_name, wa_sp] : spec.work_areas()) {
            auto solutions = wa_sp->get_solutions();

            if (solutions.empty()) {
                JUtil.warn("Work Area %s has no solutions!\n", wa_name.c_str());
                continue;
            }

            // Accumulate solutions to output json object.
            try {
                JSON work_area_json;

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
                        JUtil.error("if(team) is false!\nteam=%p in %s. Skipping...\n",
                                    team, wa_name);
                    }
                }

                output_json[wa_name] = work_area_json;
            } catch (JSON::exception const& je) {
                JSON_LOG_EXIT(je);
            }
        }
    }
};

/* public */
/* ctors */
OutputManager::OutputManager(Spec const& spec) :
    pimpl_(new_pimpl<PImpl>(*this)) {
    pimpl_->collect_output(spec);
}

/* dtors */
OutputManager::~OutputManager() {}

/* accessors */
void OutputManager::write_to_file(Options const& options,
                                  size_t const indent_size) const {
    pimpl_->write_to_file(options, indent_size);
}

}  /* elfin */