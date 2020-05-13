#ifndef SCORING_H
#define SCORING_H

#include "geometry.h"

namespace elfin
{

class WorkArea;
struct TestStat;

namespace scoring {

static double const SCORE_FLOOR = 1e-3;
static double const EPSILON = 1e-7;

inline bool almost_eq(float const a, float const b) {
  return abs(a - b) < EPSILON;
}

// Resamples two point lists of arbitrary sizes, then calls Rosetta's Kabsch in
// RMS-only mode.
float score_aligned(V3fList const& mobile,
                    V3fList const& ref);
float score_unaligned(V3fList const& mobile,
                      V3fList const& ref);

typedef decltype(score_aligned) score_func_type;

static_assert(std::is_same<score_func_type, decltype(score_unaligned)>::value,
              "score_aligned and score_unaligned must have the same signature.");

// Resamples two point lists of arbitrary sizes, then computes in-order RMS
// without Kabsch.
// float simple_rms(V3fList const& mobile, V3fList const& ref);

// A wrapper for Rosetta's Kabsch in Transform+RMS mode for point lists.
void calc_alignment(V3fList const& mobile,
                    V3fList const& ref,
                    elfin::Mat3f& rot,
                    Vector3f& tran);

/* tests */
TestStat test();

// The following functions a prefixed by underscore because they're not meant
// to be called from modules other than tests.cc.

V3fList _upsample(V3fList const& points, size_t const target);

// Implemetation of Kabsch algoritm for finding the best rotation matrix.
// ---------------------------------------------------------------------------
// mobile - mobile(i,m) are coordinates of atom m in set mobile   (input)
// ref    - ref(i,m) are coordinates of atom m in set ref         (input)
// rot    - rot(i,j) is rotation  matrix for best superposition   (output)
// tran   - tran(i) is translation vector for best superposition  (output)
// rms    - sum of w*(ux+t-y)**2 over all atom pairs              (output)
// mode   - 0:calculate rms only                                  (input)
//          1:calculate rms,u,t (takes longer)
void _rosetta_kabsch_align(V3fList const& mobile,
                           V3fList const& ref,
                           elfin::Mat3f& rot,
                           Vector3f& tran);

}  /* scroing */

} // namespace elfin

#endif /* include guard */