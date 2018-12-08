#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <deque>

#include "node.h"
#include "work_area.h"
#include "vector_map.h"
#include "checksum.h"

namespace elfin {

class NodeTeam {
private:
    /*
     * The following methods need to be implemented by derived classes because
     * the pointers owns by nodes_ and free_chains_ must be remapped and how
     * that's done depends entiredly on the derived class' nature.
     */
    NodeTeam & operator=(const NodeTeam & other) {
        NICE_PANIC("FORBIDDEN", "Implement your own copy-assign!\n");
    }
    NodeTeam(const NodeTeam & other) {
        NICE_PANIC("FORBIDDEN", "Implement your own copy-ctor!\n");
    }

protected:
    /* types */
    typedef VectorMap<Node *> Nodes;

    /* data */
    Nodes nodes_;
    FreeChainList free_chains_;

    /* ctors */

    /* modifiers */
    Node * add_member(
        const ProtoModule * prot,
        const Transform & tx = Transform());
    void remove_member(const Node * const_node);
public:
    /* ctors */
    NodeTeam();
    NodeTeam(NodeTeam && other);
    virtual NodeTeam * clone() const = 0;

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    const Nodes & nodes() const { return nodes_; }
    const FreeChainList & free_chains() const { return free_chains_; }
    const FreeChain & random_free_chain() const {
        return free_chains_.rand_item();
    }
    const ProtoLink & random_proto_link(const FreeChain & free_chain) const;
    size_t size() const { return nodes_.size(); }
    virtual float score(const WorkArea * wa) const = 0;
    virtual Crc32 checksum() const = 0;

    /* modifiers */
    NodeTeam & operator=(NodeTeam && other);
    void disperse();
    void remake(const Roulette<ProtoModule *> & mod_list);
    const Node * invite_new_member(
        const FreeChain free_chain_a, // Use a copy so deleting it won't invalid later access
        const ProtoLink & proto_link);

    /* printers */
    virtual std::string to_string() const = 0;
    virtual StrList get_node_names() const = 0;

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */