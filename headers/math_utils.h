#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include <cstdlib>
#include <vector>

#include "int_types.h"
#include "radii.h"
#include "candidate.h"

// COLLISION_MEASURE is one of {avg_all, max_heavy, max_ca}
#define COLLISION_MEASURE max_heavy

namespace elfin
{

inline bool collides(
    const uint new_id,
    const Point3f & new_com,
    ConstNodeIterator begin_node,
    ConstNodeIterator end_node)
{
    // Check collision with all nodes up to previous PAIR
    for (ConstNodeIterator itr = begin_node; itr < end_node; itr++)
    {
        const float comDist = itr->com.sq_dist_to(new_com);
        const float required_com_dist = RADII_LIST.at(itr->id).COLLISION_MEASURE +
                                        RADII_LIST.at(new_id).COLLISION_MEASURE;
        if (comDist < (required_com_dist * required_com_dist))
            return true;
    }

    return false;
}

int _test_math_utils();
} // namespace elfin

#endif /* include guard */