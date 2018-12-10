#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
private:
    /*modifiers */
    void grow(FreeChain free_chain);

    bool erode_mutate();
    bool delete_mutate();
    bool insert_mutate();
    bool swap_mutate();
    bool cross_mutate(
        const NodeTeam * father);
    bool randomize_mutate();

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
    virtual void mutate(
        MutationCounter & mt_counters,
        const NodeTeam * mother,
        const NodeTeam * father);
    virtual void randomize() { randomize_mutate(); }

    /* printers */
    virtual std::string to_string() const;
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */