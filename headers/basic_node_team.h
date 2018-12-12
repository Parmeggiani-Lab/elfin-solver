#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
private:
    /*modifiers */
    void fix_limb_transforms(Link const& arrow);

    bool erode_mutate(
        Node * tip_node = nullptr,
        long stop_after_n = -1,
        bool const regen = true);
    bool delete_mutate();
    bool insert_mutate();
    bool swap_mutate();
    bool cross_mutate(
        NodeTeam const* father);
    bool regenerate();
    bool randomize_mutate();

public:
    /* ctors */
    using NodeTeam::NodeTeam;
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(BasicNodeTeam const& other);
    virtual BasicNodeTeam * clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* accessors */
    virtual float score(WorkArea const* wa) const;
    virtual Crc32 checksum() const;

    /* modifiers */
    virtual void deep_copy_from(NodeTeam const* other);
    virtual MutationMode mutate(
        NodeTeam const* mother,
        NodeTeam const* father);
    virtual void randomize() { randomize_mutate(); }

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */