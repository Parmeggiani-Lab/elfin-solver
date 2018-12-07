#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <deque>

#include "node.h"
#include "chain_seeker.h"
#include "vector_map.h"
#include "input_manager.h"
#include "string_utils.h"
#include "work_area.h"

namespace elfin {

class NodeTeam {
protected:
    /* types */
    typedef VectorMap<const Node *> NodeVM;

    struct SeekerVectorMap : public VectorMap<ChainSeeker> {
        std::string to_string() const;
    };

    struct SeekerVMPair {
        /* data */
        SeekerVectorMap n, c;

        /* accessor */
        const SeekerVectorMap & get_vm(const TerminusType term) const {
            return const_cast<SeekerVMPair *>(this)->get_vm(term);
        }
        TerminusType get_term(const ChainSeeker & seeker) const;

        /* modifiers */
        SeekerVectorMap & get_vm(const TerminusType term);

        /* printers */
        std::string to_string() const;
    };

    /* data */
    NodeVM nodes_;
    SeekerVMPair free_seekers_;

    /* modifiers */
    static NodeVM copy_nodes_from(const NodeVM & other);
    Node * add_member(
        const ProtoModule * prot,
        const Transform & tx = Transform());
public:
    /* ctors */
    NodeTeam();
    NodeTeam(const NodeTeam & other);
    NodeTeam(NodeTeam && other);
    virtual NodeTeam * clone() const = 0;

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    const NodeVM & nodes() const { return nodes_; }
    const SeekerVMPair & free_seekers() const { return free_seekers_; }
    const ChainSeeker & random_free_seeker(TerminusType term) const;
    const ProtoLink & random_proto_link(const ChainSeeker & seeker) const;
    size_t size() const { return nodes_.items().size(); }
    virtual float score(const WorkArea & wa) const = 0;

    /* modifiers */
    NodeTeam & operator=(const NodeTeam & other);
    NodeTeam & operator=(NodeTeam && other);
    void disperse();
    void remake(const Roulette<ProtoModule *> & mod_list = XDB.basic_mods());
    const Node * invite_new_member(
        const ChainSeeker seeker_a, // Use a copy so deleting it won't invalid later access
        const ProtoLink & proto_link);

    /* printers */
    virtual StrList get_node_names() const = 0;

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */