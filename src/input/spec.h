#ifndef SPEC_H_
#define SPEC_H_

#include "json.h"
#include "../../data/Geometry.h"

namespace elfin {

// TODO: Remove inheritance
class Spec : public Points3f
{
public:
    Spec() {};
    Spec(const JSON & j);
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */