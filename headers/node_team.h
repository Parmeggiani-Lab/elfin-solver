#ifndef NODE_TEAM_H_
#define NODE_TEAM_H_

#include <deque>

#include "node.h"
#include "chain_seeker.h"
#include "vector_map.h"
#include "input_manager.h"

namespace elfin {

class NodeTeam {
protected:
    /* types */
    typedef VectorMap<const Node *> NodeVM;
    struct SeekerVM : public VectorMap<ChainSeeker> {
        std::string to_string() const {
            std::ostringstream ss;
            for (auto & sk : items_) ss << sk.to_string();
            return ss.str();
        }
    };

    struct SeekerPair {
        SeekerVM n, c;
        SeekerVM & get_vm(const TerminusType term);
        const SeekerVM & get_vm(const TerminusType term) const {
            return const_cast<SeekerPair *>(this)->get_vm(term);
        }
        TerminusType get_term(const ChainSeeker & seeker) const;
        std::string to_string() const;
    };

    /* data */
    NodeVM nodes_;
    SeekerPair free_seekers_;

    /* modifiers */
    void copy_nodes_from(const NodeVM & other);
    const Node * add_member(
        const ProtoModule * prot,
        const Transform & tx = Transform());
public:
    /* ctors */
    NodeTeam() {}
    NodeTeam(const NodeTeam & other);
    NodeTeam(NodeTeam && other);

    /* dtors */
    virtual ~NodeTeam();

    /* accessors */
    const NodeVM & nodes() const { return nodes_; }
    const SeekerPair & free_seekers() const { return free_seekers_; }
    const ChainSeeker & random_free_seeker(TerminusType term) const;
    const ProtoLink & random_proto_link(const ChainSeeker & seeker) const;
    size_t size() const { return nodes_.items().size(); }

    /* modifiers */
    NodeTeam & operator=(const NodeTeam & other);
    NodeTeam & operator=(NodeTeam && other);
    void disperse();
    void remake(const Roulette<ProtoModule *> & mod_list = XDB.basic_mods());
    const Node * invite_new_member(const ChainSeeker & seeker, const ProtoLink & proto_link);

};  /* class NodeTeam*/

}  /* elfin */

#endif  /* end of include guard: NODE_TEAM_H_ */