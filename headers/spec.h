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
    WorkAreas work_areas_;
    FixedAreas fixed_areas_;

    void map_joints();

public:
    static const char * const pg_networks_name;
    static const char * const networks_name;

    Spec() {};
    Spec(const JSON & j);
    virtual ~Spec() {};

    void parse_from_json(const JSON & j);
    WorkAreas & get_work_areas() { return work_areas_; };
    const WorkAreas & get_work_areas_cst() const { return work_areas_; };
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */