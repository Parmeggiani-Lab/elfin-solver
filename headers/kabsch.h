#ifndef KABSCH_H
#define KABSCH_H

#include <vector>

#include "geometry.h"

namespace elfin
{

class WorkArea;
struct TestStat;

namespace kabsch {

using MatXf = std::vector<std::vector<double>>;

float score(V3fList const& points, WorkArea const& wa);
float score(V3fList mobile, V3fList ref);

void _resample(
    V3fList const& ref,
    V3fList& pts);
bool _kabsch(
    V3fList const& mobile,
    V3fList const& ref,
    MatXf & rot,
    Vector3f& tran,
    double& rms,
    int mode = 1);

TestStat test();

}  /* kabsch */

} // namespace elfin

#endif /* include guard */