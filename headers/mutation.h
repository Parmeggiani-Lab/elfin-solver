#ifndef MUTATION_H_
#define MUTATION_H_

#include <unordered_map>

#include "vector_utils.h"
#include "pointer_utils.h"
#include "free_chain.h"

namespace elfin {

/* fwd dcl */
class Node;
typedef std::shared_ptr<Node> NodeSP;
class Link;

namespace mutation {

/* types */
#define FOREACH_MODES(MACRO) \
    MACRO(NONE) \
    MACRO(ERODE) \
    MACRO(DELETE) \
    MACRO(INSERT) \
    MACRO(SWAP) \
    MACRO(CROSS) \
    MACRO(REGENERATE) \
    MACRO(_ENUM_SIZE) \

GEN_ENUM_AND_STRING(Mode, ModeNames, FOREACH_MODES);

typedef std::unordered_map<Mode, size_t> Counter;
typedef Vector<Mode> ModeList;

struct DeletePoint {
    //
    // [neighbor1] <--link1-- [delete_node] --link2--> [neighbor2]
    //             dst----src               src----dst
    //             ^^^                             ^^^
    //            (src)                           (dst)
    //             --------------skipper------------->
    //
    NodeSP delete_node;
    FreeChain const src, dst;
    ProtoLink const* skipper;
    DeletePoint(
        NodeSP const& _delete_node,
        FreeChain const&  _src,
        FreeChain const&  _dst,
        ProtoLink const* _skipper) :
        src(_src),
        dst(_dst),
        delete_node(_delete_node),
        skipper(_skipper) {}
};

struct InsertPoint {
    //
    // [  node1 ] --------------link-------------> [ node2  ]
    //            src                          dst
    //
    // Each bridge has pt_link1 and pt_link2 that:
    // [  node1 ] -pt_link1-> [new_node] -pt_link2-> [ node2  ]
    //
    FreeChain const src, dst;
    FreeChain::BridgeList const bridges;
    InsertPoint(
        FreeChain const& _src,
        FreeChain const& _dst) :
        src(_src),
        dst(_dst),
        bridges(is_uninitialized(dst.node) ?
                FreeChain::BridgeList() :
                src.find_bridges(dst)) { }
};

struct SwapPoint : public InsertPoint {
    NodeSP const del_node;
    SwapPoint(
        FreeChain const& _src,
        NodeSP const& _del_node,
        FreeChain const& _dst) :
        InsertPoint(_src, _dst),
        del_node(_del_node) {}
};

struct CrossPoint {
    ProtoLink const* pt_link;
    Link const* m_arrow;
    bool m_rev;
    Link const* f_arrow;
    bool f_rev;
    CrossPoint(
        ProtoLink const* _pt_link,
        Link const* _m_arrow,
        bool const _m_rev,
        Link const* _f_arrow,
        bool const _f_rev) :
        pt_link(_pt_link),
        m_arrow(_m_arrow),
        m_rev(_m_rev),
        f_arrow(_f_arrow),
        f_rev(_f_rev) {}
};

/* free functions */
ModeList gen_mode_list();
Counter gen_counter();

static inline void bad_mode(Mode mode) {
    TRACE_PANIC(mode == mode, string_format("Bad Mutation Mode: %s\n",
                                           ModeToCStr(mode)));
}

}  /* mutation */

}  /* elfin */

#endif  /* end of include guard: MUTATION_H_ */