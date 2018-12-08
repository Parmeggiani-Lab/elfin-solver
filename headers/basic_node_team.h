#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
protected:
    /* modifiers */
    void deep_copy_from(const NodeTeam * other);

public:
    /* ctors */
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(const BasicNodeTeam & other);
    virtual BasicNodeTeam * clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* accessors */
    virtual float score(const WorkArea & wa) const;
    virtual Crc32 checksum() const;

    /* modifiers */
    BasicNodeTeam & operator=(const BasicNodeTeam & other);

    /* printers */
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */