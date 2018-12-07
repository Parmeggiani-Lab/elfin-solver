#ifndef BASIC_NODE_TEAM_H_
#define BASIC_NODE_TEAM_H_

#include "node_team.h"

namespace elfin {

class BasicNodeTeam : public NodeTeam {
private:
    /* data */

public:
    /* ctors */
    BasicNodeTeam() : NodeTeam() {}
    BasicNodeTeam(const BasicNodeTeam & other);
    BasicNodeTeam(BasicNodeTeam && other);
    virtual BasicNodeTeam * clone() const;

    /* dtors */
    virtual ~BasicNodeTeam() {}

    /* accessors */
    virtual float score(const WorkArea & wa) const;

    /* modifiers */

    /* printers */
    virtual StrList get_node_names() const;

};  /* class BasicNodeTeam*/

}  /* elfin */

#endif  /* end of include guard: BASIC_NODE_TEAM_H_ */