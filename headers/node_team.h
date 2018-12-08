#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <deque>

#include "node.h"
#include "vector_map.h"
#include "input_manager.h"
#include "string_utils.h"
#include "work_area.h"

namespace elfin {

class NodeTeam {
protected:
    /* types */
    typedef VectorMap<const Node *> NodeVM;

    /* data */
    NodeVM nodes_;
    FreeChainVM free_chains_;

    /* ctors */
    NodeTeam(const NodeTeam & other);

    /* modifiers */
    static NodeVM copy_nodes_from(const NodeVM & other);
    Node * add_member(
        const ProtoModule * prot,
        const Transform & tx = Transform());
public:
    /* ctors */
    NodeTeam();
    NodeTeam(NodeTeam && other);
    virtual NodeTeam * clone() const = 0;

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    const NodeVM & nodes() const { return nodes_; }
    const FreeChainVM & free_chains() const { return free_chains_; }
    const FreeChain & random_free_chain() const {
        return free_chains_.rand_item();
    }
    const ProtoLink & random_proto_link(const FreeChain & free_chain) const;
    size_t size() const { return nodes_.items().size(); }
    virtual float score(const WorkArea & wa) const = 0;

    /* modifiers */
    NodeTeam & operator=(const NodeTeam & other);
    NodeTeam & operator=(NodeTeam && other);
    void disperse();
    void remake(const Roulette<ProtoModule *> & mod_list = XDB.basic_mods());
    const Node * invite_new_member(
        const FreeChain free_chain_a, // Use a copy so deleting it won't invalid later access
        const ProtoLink & proto_link);

    /* printers */
    virtual StrList get_node_names() const = 0;

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */