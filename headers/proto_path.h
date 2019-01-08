#ifndef PROTO_PATH_H_
#define PROTO_PATH_H_

#include <vector>

#include "string_utils.h"

namespace elfin {

/* Fwd Decl */
class ProtoModule;
typedef ProtoModule const* PtModKey;
class ProtoLink;
typedef ProtoLink const* PtLinkKey;

struct ProtoPath : public Printable {
    /* data */
    PtModKey start;
    std::vector<PtLinkKey> links;
    
    /* ctors */
    ProtoPath(PtModKey const _start) : start(_start) {}

    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef std::vector<ProtoPath> PtPaths;

}  /* elfin */

#endif  /* end of include guard: PROTO_PATH_H_ */