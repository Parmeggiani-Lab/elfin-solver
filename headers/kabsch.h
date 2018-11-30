#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "geometry.h"
#include "candidate.h"
#include "work_area.h"

namespace elfin
{

float kabsch_score(const V3fList & points, const WorkArea & wa);
float kabsch_score(V3fList mobile, V3fList ref);

int _test_kabsch();
} // namespace elfin

#endif /* include guard */