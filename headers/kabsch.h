#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "geometry.h"
#include "work_area.h"

namespace elfin
{

namespace kabsch {

float score(V3fList const& points, WorkArea const& wa);
float score(V3fList mobile, V3fList ref);

void test(size_t& errors, size_t& tests);

}  /* kabsch */

} // namespace elfin

#endif /* include guard */