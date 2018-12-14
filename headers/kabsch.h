#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "geometry.h"
#include "candidate.h"
#include "work_area.h"

namespace elfin
{

float kabsch_score(V3fList const& points, WorkArea const* wa);
float kabsch_score(V3fList mobile, V3fList ref);

void _test_kabsch(size_t& errors, size_t& tests);

} // namespace elfin

#endif /* include guard */