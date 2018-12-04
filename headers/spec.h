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
    static const char * const pg_networks_name;
    static const char * const networks_name;

    Spec() {};
    Spec(const JSON & j);
    virtual ~Spec() {};

    void parse_from_json(const JSON & j);
    const WorkAreaMap & work_area_map() const { return work_area_map_; };
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */