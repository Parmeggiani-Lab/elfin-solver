#ifndef SPEC_H_
#define SPEC_H_

#include <unordered_map>

#include "work_area.h"
#include "fixed_area.h"
#include "json.h"

namespace elfin {

class Spec {
protected:
    /* data */
    WorkAreaMap work_areas_;
    FixedAreaMap fixed_areas_;

public:
    /* ctors */
    Spec() {}

    /* dtors */
    virtual ~Spec() {}

    /* accessors */
    WorkAreaMap const& work_areas() const {
        return work_areas_;
    }
    FixedAreaMap const& fixed_areas() const {
        return fixed_areas_;
    }

    /* modifiers */
    void parse_from_json(JSON const& json);
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */