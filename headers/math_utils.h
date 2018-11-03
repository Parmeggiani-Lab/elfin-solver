#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include <cstdlib>
#include <vector>

#include "shorthands.h"
#include "radii.h"
#include "candidate.h"

// COLLISION_MEASURE is one of {avgAll, maxHeavy, maxCA}
#define COLLISION_MEASURE maxHeavy

namespace elfin
{

inline bool
collides(const uint new_id,
         const Point3f & new_com,
         ConstNodeIterator begin_node,
         ConstNodeIterator end_node)
{
	// Check collision with all nodes up to previous PAIR
	for (ConstNodeIterator itr = begin_node; itr < end_node; itr++)
	{
		const float comDist = itr->com.sq_dist_to(new_com);
		const float requiredComDist = RADII_LIST.at(itr->id).COLLISION_MEASURE +
		                              RADII_LIST.at(new_id).COLLISION_MEASURE;
		if (comDist < (requiredComDist * requiredComDist))
			return true;
	}

	return false;
}

int _test_math_utils();
} // namespace elfin

#endif /* include guard */