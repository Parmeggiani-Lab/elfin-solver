#ifndef PROTO_PATH_H_
#define PROTO_PATH_H_

#include <vector>

namespace elfin {

/* Fwd Decl */
class ProtoModule;
typedef ProtoModule const* PtModKey;
class ProtoLink;
typedef ProtoLink const* PtLinkKey;

struct ProtoPath {
    PtModKey start;
    std::vector<PtLinkKey> links;
};

}  /* elfin */

#endif  /* end of include guard: PROTO_PATH_H_ */