#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
protected:
    /* modifiers */
    void deep_copy_from(const NodeTeam * other);
    void remove_leaf_member(Node * node);
public:
    /* ctors */
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(const BasicNodeTeam & other);
    virtual BasicNodeTeam * clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* accessors */
    virtual float score(const WorkArea * wa) const;
    virtual Crc32 checksum() const;

    /* modifiers */
    BasicNodeTeam & operator=(const BasicNodeTeam & other);
    virtual bool limb_mutate();
    virtual bool point_mutate();
    virtual bool cross_mutate(
        const NodeTeam * mother,
        const NodeTeam * father);
    virtual void grow(FreeChain free_chain);
    virtual void regrow();

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */