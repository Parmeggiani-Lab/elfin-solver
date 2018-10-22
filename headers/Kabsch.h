#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "options.h"

#include "Gene.h"

namespace elfin
{

float
kabsch_score(
    const Genes & genes,
    Points3f ref);

float
kabsch_score(
    Points3f mobile,
    Points3f ref);

int _test_kabsch(const Options &options);
} // namespace elfin

#endif /* include guard */