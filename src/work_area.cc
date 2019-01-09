#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "ui_joint_path_generator.h"

namespace elfin {

/* free */
void bad_work_type(WorkType const type) {
    throw BadWorkType(WorkTypeToCStr(type));
}

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
WorkArea::OccupantMap parse_occupants(UIJointKeys const& leaves)
{
    WorkArea::OccupantMap res;
    for (auto& leaf_joint : leaves) {
        if (leaf_joint->occupant.ui_module) {
            res.emplace(leaf_joint->occupant.ui_module->name, leaf_joint);
        }
    }

    return res;
}

WorkType parse_type(WorkArea::OccupantMap const& occupants)
{
    WorkType res = WorkType::NONE;

    size_t const n_occ_joints = occupants.size();
    TRACE(n_occ_joints > 2,
          "Parsing error: too many occupied joints (%zu) for WorkArea\n",
          n_occ_joints);

    switch (n_occ_joints) {
    case 0:
        res = WorkType::FREE;
        break;
    case 1:
        res = WorkType::HINGED;
        break;
    case 2:
        res = WorkType::DOUBLE_HINGED;
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

PtTermFinderSet parse_ptterm_profile(WorkArea::OccupantMap const& occupants) {
    PtTermFinderSet res = XDB.ptterm_finders();

    // Do for 2H case only.
    if (occupants.size() == 2) {
        // First, get UIModules and ProtoModules for the hinges.
        auto const ui_mod1 = begin(occupants)->second->occupant.ui_module;
        auto const ui_mod2 = (++begin(occupants))->second->occupant.ui_module;

        auto const mod1 = XDB.get_mod(ui_mod1->module_name);
        auto const mod2 = XDB.get_mod(ui_mod2->module_name);

        // Compute free ProtoTerms for src and dst ProtoModules.
        auto const& free_terms1 = calc_free_ptterms(mod1, ui_mod1);
        if (free_terms1.empty()) {
            throw InvalidHinge("Hinge " + ui_mod1->name + "has no free terminus.");
        }

        auto const& free_terms2 = calc_free_ptterms(mod2, ui_mod2);
        if (free_terms2.empty()) {
            throw InvalidHinge("Hinge " + ui_mod2->name + "has no free terminus.");
        }

        // Start path search from the one with fewer free ProtoTerms.
        bool const src_is_1 = free_terms1.size() < free_terms2.size();
        auto const src_mod = src_is_1 ? mod1 : mod2;
        auto const dst_mod = src_is_1 ? mod2 : mod1;
        auto const& src_terms = src_is_1 ? free_terms1 : free_terms2;
        auto const& dst_terms = src_is_1 ? free_terms2 : free_terms1;

        res = dst_mod->get_reachable_ptterms(dst_terms);

        // Remove any busy src terms from res.
        auto const src_uimod = src_is_1 ? ui_mod1 : ui_mod2;
        for (auto const& link : src_uimod->linkage) {
            auto const src_ptterm_ptr =
                &src_mod->get_chain(link.src_chain_name).get_term(link.term);

            res.erase(PtTermFinder{nullptr, 0, TermType::NONE, const_cast<ProtoTerm*>(src_ptterm_ptr)});
        }

        bool const reachable = any_of(begin(src_terms), end(src_terms),
        [&res, src_mod](auto const & ft) {
            auto const itr = res.find(
            PtTermFinder{
                nullptr,
                0,
                TermType::NONE,
                const_cast<ProtoTerm*>(&src_mod->get_term(ft))
            });
            return itr != end(res);
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

WorkArea::PathMap parse_path_map(UIJointMap const& joints,
                                 UIJointKeys const& leaves)
{
    WorkArea::PathMap res;
    TRACE(leaves.size() != 2,
          "Size of leaves not exactly 2 in work_area\n");

    auto& first_key = leaves.at(0);

    V3fList points;
    UIJointPathGenerator gen(&joints, first_key);
    while (not gen.is_done()) {
        points.emplace_back(gen.next()->tx.collapsed());
    }

    TRACE_NOMSG(points.empty());
    res.emplace(first_key, points);

    // The second path is just the reversed path.
    std::reverse(begin(points), end(points));
    auto& second_key = leaves.at(1);
    res.emplace(second_key, points);

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
WorkArea::WorkArea(
    std::string const& _name,
    JSON const& json,
    FixedAreaMap const& fam):
    /* Be careful with member ordering! */
    name(_name),
    joints(parse_joints(json, fam)),
    leaf_joints(parse_leaf_joints(joints)),
    occupants(parse_occupants(leaf_joints)),
    type(parse_type(occupants)),
    ptterm_profile(parse_ptterm_profile(occupants)),
    path_map(parse_path_map(joints, leaf_joints)),
    path_len(parse_path_len(path_map)),
    target_size(parse_target_size(path_map))
{}

/* dtors */
WorkArea::~WorkArea() {}

}  /* elfin */