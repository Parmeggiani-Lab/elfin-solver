#ifndef SPEC_H_
#define SPEC_H_

#include <unordered_map>

#include "work_area.h"
#include "fixed_area.h"
#include "json.h"

namespace elfin {

class Spec
{
protected:
    WorkAreaMap work_area_map_;
    FixedAreaMap fixed_areas_;

    void map_joints();

public:
    static char const* const pg_networks_name;
    static char const* const networks_name;

    Spec() {};
    Spec(JSON const& j);
    virtual ~Spec() {};

    void parse_from_json(JSON const& j);
    WorkAreaMap const& work_area_map() const { return work_area_map_; };
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */