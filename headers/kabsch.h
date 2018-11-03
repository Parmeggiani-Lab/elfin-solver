#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "geometry.h"

namespace elfin
{

float
kabsch_score(
    Points3f mobile,
    Points3f ref);

int _test_kabsch();
} // namespace elfin

#endif /* include guard */