#include "double_hinge_team.h"

#include <unordered_map>
#include <unordered_set>

#include "input_manager.h"
#include "path_generator.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct DoubleHingeTeam::PImpl : public PImplBase<DoubleHingeTeam> {
    using PImplBase::PImplBase;

    void find_hinge2() {
        DEBUG_NOMSG(not _.hinge_ui_joint_);
        DEBUG_NOMSG(not _.hinge_);

        auto const& omap = _.work_area_->occupied_joints;

        auto const itr = find_if(begin(omap), end(omap),
        [&](auto const & omap_pair) {
            return omap_pair.second != _.hinge_ui_joint_;
        });
        DEBUG_NOMSG(itr == end(omap));

        _.hinge_ui_joint2_ = itr->second;
    }

    void complete_by_dijkstra() {
        DEBUG_NOMSG(not _.hinge_ui_joint2_);

        std::string const& hinge2_mod_name =
            _.hinge_ui_joint2_->occupant.ui_module->module_name;

        auto const mutable_tip = _.get_tip(/*mutable_hint=*/true);
        std::string const& mutable_tip_mod_name =
            mutable_tip->prototype_->name;

        if (mutable_tip_mod_name == hinge2_mod_name) return;

        auto const src_mod = XDB.get_mod(mutable_tip_mod_name);
        auto const dst_mod = XDB.get_mod(hinge2_mod_name);

        std::unordered_set<PtModKey> frontier = { src_mod };
        std::unordered_map<PtModKey, PtLinkKey> links = { {src_mod, nullptr} };

        size_t dist = 1;
        while (not frontier.empty())
        {
            std::unordered_set<PtModKey> tmp_frontier;
            for (auto const mod : frontier) {
                for (auto const& ft : mod->free_terms()) {
                    for (auto const& link : mod->get_term(ft).links()) {
                        auto const nb = link->module;

                        // Set dist and link if nb has not been explored, or
                        // dist is smaller.
                        bool const is_new = links.find(nb) == end(links);

                        if (is_new) {
                            links[nb] = link.get();
                            tmp_frontier.insert(nb);
                        }
                    }
                }
            }

            // Stop when we reached the dst mod.
            if (tmp_frontier.find(dst_mod) != end(tmp_frontier)) {
                // Build the shortest path to dst mod.

                // Collect links (it's backwards!).
                std::vector<PtLinkKey> rev_shortest_path;

                auto curr_mod = dst_mod;
                while (curr_mod != src_mod) {
                    auto const link = links.at(curr_mod);
                    rev_shortest_path.push_back(link);
                    curr_mod = link->reverse->module;
                }

                // Check path sanity. Note the path is reversed.
                DEBUG_NOMSG(rev_shortest_path.back()->reverse->module != src_mod);
                DEBUG_NOMSG(rev_shortest_path.front()->module != dst_mod);

                // Grow the links.
                DEBUG_NOMSG(_.free_terms_.size() != 1);
                auto free_term = _.get_mutable_term();

                for (auto itr = rev_shortest_path.rbegin();
                        itr != rev_shortest_path.rend(); ++itr) {
                    _.grow_tip(free_term, *itr);
                    DEBUG_NOMSG(_.free_terms_.size() != 1);
                    free_term = _.get_mutable_term();
                }

                return;
            }

            frontier = tmp_frontier;
            dist++;
        }

        {
            std::ostringstream oss;
            oss << "Second hinge not reached!\n";
            oss << "Frontier: \n";
            for (auto const mod : frontier) {
                oss << "  " << mod->name << "\n";
            }
            oss << "Links: \n";
            for (auto const& [mod, link] : links) {
                oss << "  " << mod->name << ":" << link << "\n";
            }

            throw ShouldNotReach(oss.str());
        }

    }
};

/* protected */
/* accessors */
DoubleHingeTeam* DoubleHingeTeam::virtual_clone() const {
    return new DoubleHingeTeam(*this);
}

void DoubleHingeTeam::postprocess_json(JSON& output) const {
    HingeTeam::postprocess_json(output);  // Remove first hinge.

    if (output.size() > 0) {
        // Skip last *supposed* hinge.
        output.erase(output.size() - 1);
    }
}

/* modifiers */
void DoubleHingeTeam::reset() {
    HingeTeam::reset();
    hinge_ui_joint2_ = nullptr;

    pimpl_->find_hinge2();
}

void DoubleHingeTeam::virtual_copy(NodeTeam const& other) {
    try {  // Catch bad cast
        HingeTeam::operator=(
            static_cast<DoubleHingeTeam const&>(other));
    }
    catch (std::bad_cast const& e) {
        TRACE_NOMSG("Bad cast\n");
    }
}
void DoubleHingeTeam::evaluate() {
    // Run djistrak to complete the mutable end if team does not end in second
    // hinge.
    pimpl_->complete_by_dijkstra();

    HingeTeam::evaluate();
}

void DoubleHingeTeam::virtual_implement_recipe(
    tests::Recipe const& recipe,
    FirstLastNodeKeyCallback const& _postprocessor,
    Transform const& shift_tx)
{
    // Partial reset.
    hinge_ui_joint2_ = nullptr;

    FirstLastNodeKeyCallback const& postprocessor =
    [&](NodeKey const first_node, NodeKey const last_node) {
        pimpl_->find_hinge2();

        if (_postprocessor) {
            _postprocessor(first_node, last_node);
        }
    };

    HingeTeam::virtual_implement_recipe(recipe, postprocessor, shift_tx);
}

/* public */
/* ctors */
DoubleHingeTeam::DoubleHingeTeam(WorkArea const* wa) :
    HingeTeam(wa),
    pimpl_(new_pimpl<PImpl>(*this))
{
    DEBUG_NOMSG(wa->ptterm_profile.empty());
    pimpl_->find_hinge2();
}

DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam const& other) :
    DoubleHingeTeam(other.work_area_) {
    HingeTeam::operator=(other);

}
DoubleHingeTeam::DoubleHingeTeam(DoubleHingeTeam&& other) :
    DoubleHingeTeam(other.work_area_) {
    HingeTeam::operator=(std::move(other));
}

/* dtors */
DoubleHingeTeam::~DoubleHingeTeam() {}

/* modifiers */
DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam const& other) {
    HingeTeam::operator=(other);
    hinge_ui_joint2_ = other.hinge_ui_joint2_;
    return *this;
}

DoubleHingeTeam& DoubleHingeTeam::operator=(DoubleHingeTeam && other) {
    HingeTeam::operator=(std::move(other));
    std::swap(hinge_ui_joint2_, other.hinge_ui_joint2_);
    return *this;
}

}  /* elfin */