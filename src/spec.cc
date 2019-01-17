#include "spec.h"

#include <unordered_map>
#include <deque>

#include "fixed_area.h"
#include "debug_utils.h"
#include "options.h"
#include "json.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct Spec::PImpl : public PImplBase<Spec> {
    using PImplBase::PImplBase;

    /* data */
    WorkPackages work_packages_;
    FixedAreaMap fixed_areas_;

    /* modifiers */
    void digest_network(std::string const & name, JSON const & network) {
        fixed_areas_.emplace(
            name,
            std::make_unique<FixedArea>(name, network));
    }

    void parse(Options const & options) {
        work_packages_.clear();
        fixed_areas_.clear();

        auto const& spec_file = options.spec_file;
        JUtil.info("Parsing spec file: %s\n", spec_file.c_str());

        PANIC_IF(spec_file.empty(),
                 BadArgument("No input spec file provided.\n"));

        PANIC_IF(not JUtil.file_exists(spec_file.c_str()),
                 BadArgument("Input file \"" + spec_file + "\" does not exist.\n"));

        try {
            auto const& spec_json = parse_json(spec_file);
            auto const& networks_json = spec_json.at("networks");
            auto const& pg_networks_json = spec_json.at("pg_networks");

            JUtil.info("Input spec has %zu networks and %zu pg_networks.\n",
                       networks_json.size(),
                       pg_networks_json.size());

            // Initialize fixed areas first so work areas can refer to fixed
            // modules as occupants or hinges.
            for (auto& [name, json] : networks_json.items()) {
                digest_network(name, json);
            }

            for (auto& [name, json] : pg_networks_json.items()) {
                work_packages_.emplace_back(
                    std::make_unique<WorkPackage>(name, fixed_areas_, json));
            }
        } catch (JSON::exception const& je) {
            JSON_LOG_EXIT(je);
        }

        JUtil.info("Parsed %zu fixed areas and %zu work packages\n",
                   fixed_areas_.size(), work_packages_.size());
    }
};

/* public */
/* ctors */
Spec::Spec(Options const& options) :
    pimpl_(new_pimpl<PImpl>(*this)) {
    pimpl_->parse(options);
}

Spec::Spec(Spec&& other) {
    this->operator=(std::move(other));
}

/* dtors */
Spec::~Spec() {}

/* accessors */
WorkPackages const& Spec::work_packages() const {
    return pimpl_->work_packages_;
}

/* modifiers */
Spec& Spec::operator=(Spec&& other) {
    if (this != &other) {
        pimpl_->work_packages_.clear();
        pimpl_->fixed_areas_.clear();

        std::swap(pimpl_->work_packages_, other.pimpl_->work_packages_);
        std::swap(pimpl_->fixed_areas_, other.pimpl_->fixed_areas_);
    }

    return *this;
}

void Spec::solve_all() {
    // Solve each work package.
    for (auto& wp : pimpl_->work_packages_) {
        wp->solve();
    }
}

}  /* elfin */