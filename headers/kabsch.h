#ifndef KABSCH_H
#define KABSCH_H

#include "geometry.h"

namespace elfin
{

class WorkArea;
struct TestStat;

namespace kabsch {

float score(V3fList const& mobile, V3fList const& ref);

void calc_alignment(
    V3fList const& mobile,
    V3fList const& ref,
    elfin::Mat3f& rot,
    Vector3f& tran,
    float& rms);

TestStat test();

// The following functions a prefixed by underscore because they're not meant
// to be called from other modules except for testing.

V3fList _resample(V3fList const& ref, V3fList const& pts);

// Implemetation of Kabsch algoritm for finding the best rotation matrix.
// ---------------------------------------------------------------------------
// mobile - mobile(i,m) are coordinates of atom m in set mobile   (input)
// ref    - ref(i,m) are coordinates of atom m in set ref         (input)
// rot    - rot(i,j) is rotation  matrix for best superposition   (output)
// tran   - tran(i) is translation vector for best superposition  (output)
// rms    - sum of w*(ux+t-y)**2 over all atom pairs              (output)
// mode   - 0:calculate rms only                                  (input)
//          1:calculate rms,u,t (takes longer)
void _rosetta_kabsch_align(
    V3fList const& mobile,
    V3fList const& ref,
    elfin::Mat3f& rot,
    Vector3f& tran,
    float& rms);

float _rosetta_kabsch_rms(
    V3fList const& mobile,
    V3fList const& ref);

}  /* kabsch */

} // namespace elfin

#endif /* include guard */