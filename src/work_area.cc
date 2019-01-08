#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "fixed_area.h"
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
WorkArea::OccupantMap parse_occupant_map(UIJointKeys const& leaves)
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
    FreeTerms res = ptmod->free_terms();

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

PtPaths parse_proto_paths(WorkArea::OccupantMap const& occupants) {
    PtPaths res;

    // Do for 2H case only.
    if (occupants.size() == 2) {
        // Do DFS to collect all PtPaths from hinge1 to hinge2.

        // First, get UIModules and ProtoModules for the hinges.
        auto const ui_mod1 = begin(occupants)->second->occupant.ui_module;
        auto const ui_mod2 = (++begin(occupants))->second->occupant.ui_module;

        auto const mod1 = XDB.get_mod(ui_mod1->module_name);
        auto const mod2 = XDB.get_mod(ui_mod2->module_name);

        // Compute free ProtoTerms for src and dst ProtoModules.
        auto const& fterms1 = calc_free_ptterms(mod1, ui_mod1);
        if (fterms1.empty()) {
            throw InvalidHinge("Hinge " + ui_mod1->name + "has no free terminus.");
        }

        auto const& fterms2 = calc_free_ptterms(mod2, ui_mod2);
        if (fterms2.empty()) {
            throw InvalidHinge("Hinge " + ui_mod2->name + "has no free terminus.");
        }

        // Start path search from the one with fewer free ProtoTerms.
        bool const src_is_1 = fterms1.size() < fterms2.size();
        auto const src_mod = src_is_1 ? mod1 : mod2;
        auto const dst_mod = src_is_1 ? mod2 : mod1;
        auto const& src_fterms = src_is_1 ? fterms1 : fterms2;
        auto const& dst_fterms = src_is_1 ? fterms2 : fterms1;

        for (auto const& sft : src_fterms) {
            auto const& paths = src_mod->find_paths(sft, dst_mod, dst_fterms);
            res.insert(end(res), begin(paths), end(paths));
        }

        if (res.empty()) {
            throw InvalidHinge("No paths exist between hinges " +
                               ui_mod1->name + " and " + ui_mod2->name + ".");
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
    occupant_map(parse_occupant_map(leaf_joints)),
    type(parse_type(occupant_map)),
    proto_paths(parse_proto_paths(occupant_map)),
    path_map(parse_path_map(joints, leaf_joints)),
    path_len(parse_path_len(path_map)),
    target_size(parse_target_size(path_map))
{}

/* dtors */
WorkArea::~WorkArea() {}

}  /* elfin */