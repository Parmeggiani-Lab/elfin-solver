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

    /* modifiers */
    void map_joints();
    void release_resources();

public:
    /* data */
    static char const* const pg_networks_name;
    static char const* const networks_name;

    /* ctors */
    Spec() {}
    Spec(Spec const& other) = delete;

    /* dtors */
    virtual ~Spec();

    /* accessors */
    WorkAreaMap const& work_areas() const {
        return work_areas_;
    }

    /* modifiers */
    Spec& operator=(Spec const& other) = delete;
    void parse_from_json(JSON const& j);
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */