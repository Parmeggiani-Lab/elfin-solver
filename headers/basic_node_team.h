#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
public:
    /* ctors */
    using NodeTeam::NodeTeam;
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(const BasicNodeTeam & other);
    virtual BasicNodeTeam * clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* accessors */
    virtual float score(const WorkArea * wa) const;
    virtual Crc32 checksum() const;

    /* modifiers */
    virtual void deep_copy_from(const NodeTeam * other);
    virtual bool limb_mutate();
    virtual bool point_mutate();
    virtual bool cross_mutate(
        const NodeTeam * mother,
        const NodeTeam * father);
    virtual void grow(FreeChain free_chain);
    virtual void randomize();

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */