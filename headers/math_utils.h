#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include <cstdlib>
#include <vector>

#include "shorthands.h"
#include "radii.h"
#include "Gene.h"

// COLLISION_MEASURE is one of {avgAll, maxHeavy, maxCA}
#define COLLISION_MEASURE maxHeavy

namespace elfin
{

inline bool
collides(const uint newId,
         const Point3f & newCOM,
         ConstGeneIterator beginGene,
         ConstGeneIterator endGene,
         const RadiiList & radiiList)
{
	// Check collision with all nodes up to previous PAIR
	for (ConstGeneIterator itr = beginGene; itr < endGene; itr++)
	{
		const float comDist = itr->com().sq_dist_to(newCOM);
		const float requiredComDist = radiiList.at(itr->nodeId()).COLLISION_MEASURE +
		                              radiiList.at(newId).COLLISION_MEASURE;
		if (comDist < (requiredComDist * requiredComDist))
			return true;
	}

	return false;
}

int _test_math_utils();
} // namespace elfin

#endif /* include guard */