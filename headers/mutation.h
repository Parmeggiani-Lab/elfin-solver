#ifndef MUTATION_H_
#define MUTATION_H_

#include <unordered_map>

#include "free_chain.h"

namespace elfin {

/* Fwd Decl */
class Node;
typedef Node const* NodeKey;
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
typedef std::vector<Mode> ModeList;

struct DeletePoint {
    //
    // [neighbor1] <--link1-- [delete_node] --link2--> [neighbor2]
    //             dst----src               src----dst
    //             ^^^                             ^^^
    //            (src)                           (dst)
    //             --------------skipper------------->
    //
    NodeKey const       delete_node;
    FreeChain const     src;
    FreeChain const     dst;
    ProtoLink const*    skipper;
    DeletePoint(NodeKey const _delete_node,
                FreeChain const&  _src,
                FreeChain const&  _dst,
                ProtoLink const* _skipper) :
        delete_node(_delete_node),
        src(_src),
        dst(_dst),
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
    FreeChain const             src;
    FreeChain const             dst;
    FreeChain::BridgeList const bridges;
    InsertPoint(FreeChain const& _src,
                FreeChain const& _dst) :
        src(_src),
        dst(_dst),
        bridges(dst.node ?
                src.find_bridges(dst) :
                FreeChain::BridgeList()) { }
};

struct SwapPoint : public InsertPoint {
    NodeKey const del_node;
    SwapPoint(FreeChain const& _src,
              NodeKey const _del_node,
              FreeChain const& _dst) :
        InsertPoint(_src, _dst),
        del_node(_del_node) {}
};

struct CrossPoint {
    ProtoLink const* const  pt_link;
    Link const* const       m_arrow;
    Link const* const       f_arrow;
    bool const              f_rev;
    CrossPoint(ProtoLink const* const _pt_link,
               Link const* const _m_arrow,
               Link const* const _f_arrow,
               bool const _f_rev) :
        pt_link(_pt_link),
        m_arrow(_m_arrow),
        f_arrow(_f_arrow),
        f_rev(_f_rev) {}
};

/* free functions */
ModeList gen_mode_list();
Counter gen_counter();

static inline void bad_mode(Mode mode) {
    TRACE(mode == mode, "Bad Mutation Mode: %s\n", ModeToCStr(mode));
}

}  /* mutation */

}  /* elfin */

#endif  /* end of include guard: MUTATION_H_ */