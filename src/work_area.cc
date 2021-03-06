#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "ui_joint_path_generator.h"
#include "evolution_solver.h"
#include "priv_impl.h"

namespace elfin {

/* private */
struct WorkArea::PImpl : public PImplBase<WorkArea> {
    using PImplBase::PImplBase;

    /* data */
    EvolutionSolver solver_;
    TeamSPMaxHeap solutions_;

    /* accessors */
    TeamPtrMinHeap solutions_to_minheap() {
        TeamPtrMinHeap res;
        TeamSPMaxHeap tmp;

        while (not solutions_.empty()) {
            auto node_sp = solutions_.top_and_pop();
            res.push(node_sp.get());
            tmp.push(std::move(node_sp));
        }

        // Transfer back.
        while (not tmp.empty()) {
            solutions_.push(tmp.top_and_pop());
        }

        return res;
    }

    /* accessors */
    PathMap parse_path_map() const {
        PathMap res;
        V3fList fwd_path;
        TRACE(_.leaf_joints.size() != 2,
              "Expecting 2 leaves but got %zu in work_area %s.\n",
              _.leaf_joints.size(), _.name.c_str());

        auto& first_key = _.leaf_joints.at(0);

        UIJointPathGenerator gen(&_.joints, first_key);
        while (not gen.is_done()) {
            fwd_path.emplace_back(gen.next()->tx.collapsed());
        }

        TRACE_NOMSG(fwd_path.empty());
        res.emplace(first_key, fwd_path);
        res.emplace(_.leaf_joints.at(1),
                    V3fList(rbegin(fwd_path), rend(fwd_path)));

        return res;
    }

    /* modifiers */
    void solve() {
        solver_.run(/*work_area=*/_, solutions_);
    }
};

/* data initializers */
UIJointMap parse_joints(JSON const& json,
                        FixedAreaMap const& fam)
{
    UIJointMap res;
    for (auto& [joint_name, joint_json] : json.items()) {
        res.emplace(
            joint_name,
            std::make_unique<UIJoint>(joint_name, joint_json, fam));
    }
    return res;
}

UIJointKeys parse_leaf_joints(UIJointMap const& joints)
{
    UIJointKeys res;
    for (auto& [name, joint] : joints) {
        if (joint->neighbors.size() == 1) {
            res.push_back(joint.get());
        }
    }
    return res;
}

// Occupied joints are supposed to be a subset of leaves.
WorkArea::NamedJoints parse_occupied_joints(UIJointKeys const& leaves)
{
    WorkArea::NamedJoints res;
    for (auto leaf : leaves) {
        if (leaf->occupant.ui_module) {
            res.emplace(leaf->occupant.ui_module->name, leaf);
        }
    }

    return res;
}

WorkType parse_type(WorkArea::NamedJoints const& occupied_joints)
{
    WorkType res = WorkType::NONE;

    size_t const n_occ_joints = occupied_joints.size();
    TRACE(n_occ_joints > 2,
          "Parsing error: too many occupied joints (%zu) for WorkArea\n",
          n_occ_joints);

    switch (n_occ_joints) {
    case 0:
        res = WorkType::FREE;
        break;
    case 1:
        res = WorkType::HINGE;
        break;
    case 2:
        res = WorkType::DOUBLE_HINGE;
        break;
    default:
        TRACE("Too many occupied joints",
              "Expecting 0, 1, or 2 but got %zu\n",
              n_occ_joints);
    }

    return res;
}

FreeTerms calc_free_ptterms(PtModKey const ptmod, UIModKey const uimod) {
    auto res = ptmod->free_terms();

    if (ptmod->name != uimod->module_name) {
        throw BadArgument("ProtoModule name differs from UIModule module name.");
    }

    for (auto const& link : uimod->linkage) {
        size_t const src_chain_id =
            ptmod->get_chain_id(link.src_chain_name);

        res.erase(remove_if(begin(res), end(res),
        [&](auto const & ft) {
            return ft.term == link.term and
                   ft.chain_id == src_chain_id;
        }),
        end(res));
    }

    return res;
}

PtTermFinderSet parse_ptterm_profile(WorkArea::NamedJoints const& occupied_joints) {
    PtTermFinderSet res = XDB.ptterm_finders();

    // Do for 2H case only.
    if (occupied_joints.size() == 2) {
        // First, get UIModules and ProtoModules for the hinges.
        auto const ui_mod1 =
            begin(occupied_joints)->second->occupant.ui_module;
        auto const ui_mod2 =
            (++begin(occupied_joints))->second->occupant.ui_module;

        auto const mod1 = XDB.get_mod(ui_mod1->module_name);
        auto const mod2 = XDB.get_mod(ui_mod2->module_name);

        // Compute free ProtoTerms for src and dst ProtoModules.
        auto const& free_terms1 = calc_free_ptterms(mod1, ui_mod1);
        if (free_terms1.empty()) {
            throw InvalidHinge("Hinge " + ui_mod1->name + " has no free terminus.");
        }

        auto const& free_terms2 = calc_free_ptterms(mod2, ui_mod2);
        if (free_terms2.empty()) {
            throw InvalidHinge("Hinge " + ui_mod2->name + " has no free terminus.");
        }

        // Start path search from the one with fewer free ProtoTerms.
        bool const src_is_1 = free_terms1.size() < free_terms2.size();
        auto const src_mod = src_is_1 ? mod1 : mod2;
        auto const dst_mod = src_is_1 ? mod2 : mod1;
        auto const& src_terms = src_is_1 ? free_terms1 : free_terms2;
        auto const& dst_terms = src_is_1 ? free_terms2 : free_terms1;

        res = dst_mod->get_reachable_ptterms(dst_terms);

        // Suppress dead ends i.e. ProtoModule with only one active terminus.
        auto const in_res = [&res](PtTermKey const key) {
            auto const itr = res.find(PtTermFinder(nullptr, 0, TermType::NONE, key));
            return itr != end(res);
        };

        for (auto const& mod : XDB.all_mods()) {
            // Dst mod and src mod can dead end; it's ok.
            if (mod.get() == dst_mod or
                    mod.get() == src_mod) continue;

            size_t active_terms = 0;

            PtTermFinderSet mod_finders;
            for (auto const& chain : mod->chains()) {
                size_t const chain_id = mod->get_chain_id(chain.name);

                mod_finders.emplace(mod.get(), chain_id, TermType::N, &chain.n_term());
                if (in_res(&chain.n_term()))
                    active_terms++;

                mod_finders.emplace(mod.get(), chain_id, TermType::C, &chain.c_term());
                if (in_res(&chain.c_term()))
                    active_terms++;
            }

            // Any ProtoModule with fewer than 2 active termini is a dead end.
            // It's only posible to enter but not exit
            if (active_terms < 2) {
                for (auto const& finder : mod_finders) {
                    res.erase(finder);
                }
                active_terms = 0;
            }
        }

        // Verify that src mod is reachable from dst mod.
        bool const reachable = any_of(begin(src_terms), end(src_terms),
        [&in_res, src_mod](auto const & ft) {
            return in_res(&src_mod->get_term(ft));
        });

        if (not reachable) {
            std::ostringstream oss;
            auto const print_free_terms = [&oss](PtModKey const mod, FreeTerms const & fts) {
                oss << mod->name << " free terms:\n";
                for (auto const ft : fts) {
                    oss << "  " << mod->chains().at(ft.chain_id).name;
                    oss << "(id=" << ft.chain_id << ")";
                    oss << ":" << TermTypeToCStr(ft.term) << "\n";
                }
            };

            print_free_terms(src_mod, src_terms);
            print_free_terms(dst_mod, dst_terms);
            throw InvalidHinge("No paths exist between hinges " +
                               ui_mod1->name + " and " + ui_mod2->name + ".\n" + oss.str());
        }
    }

    return res;
}

size_t parse_path_len(WorkArea::PathMap const& paths) {
    auto const& [ui_key, path] = *begin(paths);
    return path.size();
}

size_t parse_target_size(WorkArea::PathMap const& paths)
{
    // Calculate expected length as sum of point
    // displacements over avg pair module distance

    float sum_dist = 0.0f;
    auto const& [ui_key, path] = *(begin(paths));
    for (auto itr = begin(path) + 1; itr != end(path); ++itr) {
        sum_dist += (itr - 1)->dist_to(*itr);
    }

    // Add one to nubmer of segments.
    size_t const expected_len =
        1 + round(sum_dist / OPTIONS.avg_pair_dist);

    size_t const res = expected_len + OPTIONS.len_dev;

    return res;
}

/* public */
/* ctors */
WorkArea::WorkArea(std::string const& _name,
                   JSON const& json,
                   FixedAreaMap const& fam):
    /* Be careful with member ordering! */
    pimpl_(new_pimpl<PImpl>(*this)),
    name(_name),
    joints(parse_joints(json, fam)),
    leaf_joints(parse_leaf_joints(joints)),
    occupied_joints(parse_occupied_joints(leaf_joints)),
    type(parse_type(occupied_joints)),
    ptterm_profile(parse_ptterm_profile(occupied_joints)),
    path_map(pimpl_->parse_path_map(/*relies on joints, leaf_joints*/)),
    path_len(parse_path_len(path_map)),
    target_size(parse_target_size(path_map))
{
    PANIC_IF(joints.empty(),
             ShouldNotReach("Work Area \"" + name + "\" has no joints. " +
                            "Error in parsing or input spec, maybe?"));
}

/* dtors */
WorkArea::~WorkArea() {}

/* accessors */
TeamPtrMinHeap WorkArea:: make_solution_minheap() const {
    return pimpl_->solutions_to_minheap();
}

/* modifiers */
void WorkArea::solve() {
    pimpl_->solve();
}

}  /* elfin */