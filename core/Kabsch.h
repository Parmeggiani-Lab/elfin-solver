#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "../data/TypeDefs.h"
#include "../data/Gene.h"

namespace elfin
{

float
kabschScore(
    const Genes & genes,
    Points3f ref);

float
kabschScore(
    Points3f mobile,
    Points3f ref);

int _testKabsch();
} // namespace elfin

#endif /* include guard */