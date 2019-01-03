#include "work_area.h"

#include <tuple>
#include <sstream>

#include "debug_utils.h"
#include "input_manager.h"
#include "fixed_area.h"
#include "ui_joint_path_generator.h"

namespace elfin {

/* free */
void bad_work_type(WorkType type) {
    TRACE(WorkTypeToCStr(type), "Bad WorkType: %s\n", WorkTypeToCStr(type));
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

UIJointKeys parse_leaf_joints(UIJointMap const& _joints)
{
    UIJointKeys res;
    for (auto& [name, joint] : _joints) {
        if (joint->neighbors.size() == 1) {
            res.push_back(joint.get());
        }
    }
    return res;
}

using PtModKey = ProtoModule const*;
using PtModVisitMap = std::unordered_map<PtModKey, bool>;
bool has_path_to(PtModKey const target_mod,
                 PtModKey const prev_mod,
                 PtModVisitMap& visited)
{
    DEBUG_NOMSG(not target_mod);
    DEBUG_NOMSG(not prev_mod);

    // DFS search for target_mod.
    visited[prev_mod] = true;

    auto check_ptterm = [&](ProtoTerm const & ptterm) {
        return any_of(begin(ptterm.links()), end(ptterm.links()),
        [&](auto const & ptlink) {
            auto const dst = ptlink->module_;
            return dst == target_mod or
                   (not visited[dst] and has_path_to(target_mod, dst, visited));
        });
    };

    for (auto const& ptchain : prev_mod->chains()) {
        if (check_ptterm(ptchain.n_term()) or
                check_ptterm(ptchain.c_term())) {
            return true;
        }
    }

    return false;
}

// Occupied joints are supposed to be a subset of leaves.
WorkArea::OccupantMap parse_occupant_map(UIJointKeys const& _leaf_joints)
{
    //
    // THIS TEST DOES NOT TAKE INTO ACCOUNT TERMINUS DIRECTIONS YET!!!
    //
    WorkArea::OccupantMap res;
    for (auto& leaf_joint : _leaf_joints) {
        if (leaf_joint->occupant.ui_module) {
            res.emplace(leaf_joint->occupant.ui_module->name, leaf_joint);
        }
    }

    if (res.size() == 2) {
        // Do DFS to determine whether first hinge actually has a path to
        // second hinge.
        auto const& hinge_name1 =
            begin(res)->second->occupant.ui_module->module_name;
        auto const& hinge_name2 =
            (++begin(res))->second->occupant.ui_module->module_name;

        auto const& mimap = XDB.mod_idx_map();
        size_t const hinge_id1 = mimap.at(hinge_name1);
        size_t const hinge_id2 = mimap.at(hinge_name2);

        auto const mod_key1 = XDB.all_mods()[hinge_id1].get();
        auto const mod_key2 = XDB.all_mods()[hinge_id2].get();

        PtModVisitMap visited;
        for (auto const& mod : XDB.all_mods()) {
            visited.emplace(mod.get(), false);
        }

        if (not has_path_to(mod_key2, mod_key1, visited)) {
            throw InvalidHingeException(
                "No path between " + hinge_name1 +
                " and " + hinge_name2);
        }
    }

    return res;
}

WorkType parse_type(UIJointMap const& _joints,
                    WorkArea::OccupantMap const& _occupant_map)
{
    WorkType res = WorkType::NONE;

    size_t const n_occ_joints = _occupant_map.size();
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

WorkArea::PathMap parse_path_map(UIJointMap const& _joints,
                                 UIJointKeys const& _leaf_joints)
{
    WorkArea::PathMap res;
    TRACE(_leaf_joints.size() != 2,
          "Size of _leaf_joints not exactly 2 in work_area\n");

    auto& first_key = _leaf_joints.at(0);

    V3fList points;
    UIJointPathGenerator gen(&_joints, first_key);
    while (not gen.is_done()) {
        points.emplace_back(gen.next()->tx.collapsed());
    }

    TRACE_NOMSG(points.empty());
    res.emplace(first_key, points);

    // The second path is just the reversed path.
    std::reverse(begin(points), end(points));
    auto& second_key = _leaf_joints.at(1);
    res.emplace(second_key, points);

    return res;
}

size_t parse_path_len(WorkArea::PathMap const& _path_map) {
    auto const& [ui_key, path] = *begin(_path_map);
    return path.size();
}

size_t parse_target_size(WorkArea::PathMap const& _path_map)
{
    // Calculate expected length as sum of point
    // displacements over avg pair module distance

    float sum_dist = 0.0f;
    auto const& [ui_key, path] = *(begin(_path_map));
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
    type(parse_type(joints, occupant_map)),
    path_map(parse_path_map(joints, leaf_joints)),
    path_len(parse_path_len(path_map)),
    target_size(parse_target_size(path_map))
{}

/* dtors */
WorkArea::~WorkArea() {}

}  /* elfin */