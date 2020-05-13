#include "scoring.h"

#include <numeric>

#include "debug_utils.h"
#include "test_data.h"

namespace elfin {

namespace scoring {


float aligned_rms(V3fList const& mobile,
                  V3fList const& ref)
{
    size_t const n = mobile.size();
    size_t const ref_n = ref.size();

    // Check sample sizes.
    DEBUG_NOMSG(n < 1);
    DEBUG_NOMSG(ref_n < 1);
    DEBUG_NOMSG(n != ref_n);

    double e0 = 0.0f;
    double h = 0.0f;
    double g = 0.0f;
    double cth = 0.0f;
    double sth = 0.0f;
    double sqrth = 0.0f;
    double det = 0.0f;
    double sigma = 0.0f;
    double xc[3] = { 0 }, yc[3] = { 0 };
    double r[3][3] = {0};
    double e[3] = {0};
    double rr[6] = {0};
    double sqrt3 = 1.73205080756888;

    // Compute centers for point vectors x, y.
    for (size_t i = 0; i < n; ++i) {
        xc[0] += mobile[i][0];
        xc[1] += mobile[i][1];
        xc[2] += mobile[i][2];

        yc[0] += ref[i][0];
        yc[1] += ref[i][1];
        yc[2] += ref[i][2];
    }

    for (size_t i = 0; i < 3; ++i) {
        xc[i] = xc[i] / n;
        yc[i] = yc[i] / n;
    }

    // Compute e0 and matrix r.
    for (size_t m = 0; m < n; m++) {
        for (size_t i = 0; i < 3; ++i) {
            double const d = ref[m][i] - yc[i];
            e0 += (mobile[m][i] - xc[i]) * (mobile[m][i] - xc[i]) + \
                  (d * d);
            for (size_t j = 0; j < 3; j++) {
                r[i][j] += d * (mobile[m][j] - xc[j]);
            }
        }
    }

    // Compute determinat of matrix r.
    det = r[0][0] * ( r[1][1] * r[2][2] - r[1][2] * r[2][1] )
          - r[0][1] * ( r[1][0] * r[2][2] - r[1][2] * r[2][0] )
          + r[0][2] * ( r[1][0] * r[2][1] - r[1][1] * r[2][0] );
    sigma = det;

    // Compute tras(r)*r.
    {
        size_t m = 0;
        for (size_t j = 0; j < 3; j++) {
            for (size_t i = 0; i <= j; ++i) {
                rr[m++] = r[0][i] * r[0][j] + r[1][i] * r[1][j] + r[2][i] * r[2][j];
            }
        }
    }

    double const spur = (rr[0] + rr[2] + rr[5]) / 3.0;
    double const cof = (((((rr[2] * rr[5] - rr[4] * rr[4]) + rr[0] * rr[5])
                          - rr[3] * rr[3]) + rr[0] * rr[2]) - rr[1] * rr[1]) / 3.0;
    det = det * det;

    for (size_t i = 0; i < 3; ++i) {
        e[i] = spur;
    }

    if (spur > 0) {
        h = (spur * spur) - cof;
        g = (spur * cof - det) / 2.0 - spur * h;

        if (h > 0) {
            sqrth = sqrt(h);
            double d = h * h * h - g * g;
            if (d < 0.0 ) d = 0.0;
            d = atan2( sqrt(d), -g ) / 3.0;
            cth = sqrth * cos(d);
            sth = sqrth * sqrt3 * sin(d);
            e[0] = (spur + cth) + cth;
            e[1] = (spur - cth) + sth;
            e[2] = (spur - cth) - sth;
        }  //h>0
    }

    // Compute rms.
    for (size_t i = 0; i < 3; ++i) {
        if (e[i] < 0 ) e[i] = 0;
        e[i] = sqrt( e[i] );
    }

    double const d = (sigma < 0.0 ? -1 : 1) * e[2] + e[1] + e[0];
    float const rms = e0 - d - d;

    if (rms < 0.0 ) {
        return 0.0;
    }

    return rms;
}

float unaligned_rms(V3fList const& mobile,
                    V3fList const& ref)
{
    size_t const n = mobile.size();
    size_t const ref_n = ref.size();

    // Check sample sizes.
    DEBUG_NOMSG(n < 1);
    DEBUG_NOMSG(ref_n < 1);
    DEBUG_NOMSG(n != ref_n);

    float const rms = sqrt(std::inner_product(
                               mobile.begin(),
                               mobile.end(),
                               ref.begin(),
                               (double) 0.0,
    std::plus<>(), [](auto a, auto b) {
        auto const e = a.sq_dist_to(b);
        return e * e;
    }) / n);

    return rms;
}

void calc_alignment(V3fList const& mobile,
                    V3fList const& ref,
                    elfin::Mat3f& rot,
                    Vector3f& tran)
{
    // Upsample if needed. Also do a bit of checking to avoid unnecessary copying
    size_t const ref_size = ref.size();
    size_t const mobile_size = mobile.size();
    if (ref_size < mobile_size)
    {
        return _rosetta_kabsch_align(mobile, _upsample(ref, mobile_size), rot, tran);
    }
    else if (mobile_size < ref_size)
    {
        return _rosetta_kabsch_align(_upsample(mobile, ref_size), ref, rot, tran);
    }
    else
    {
        return _rosetta_kabsch_align(mobile, ref, rot, tran);
    }
}

float _score(V3fList const& mobile,
             V3fList const& ref,
             float (*scoring_func)(V3fList const&, V3fList const&))
{
    if (mobile.size() == 1 and ref.size() == 1)
        return 0;

    if (mobile.size() <= 1 or ref.size() <= 1)
        return INFINITY;

    // Upsample if needed. Also do a bit of checking to avoid unnecessary copying
    size_t const ref_size = ref.size();
    size_t const mobile_size = mobile.size();
    if (ref_size < mobile_size)
    {
        return scoring_func(mobile, _upsample(ref, mobile_size));
    }
    else if (mobile_size < ref_size)
    {
        return scoring_func(_upsample(mobile, ref_size), ref);
    }
    else
    {
        return scoring_func(mobile, ref);
    }
}

float score_aligned(V3fList const& mobile, V3fList const& ref) {
    return _score(mobile, ref, aligned_rms);
}

float score_unaligned(V3fList const& mobile, V3fList const& ref) {
    return _score(mobile, ref, unaligned_rms);
}

struct SampleSegment
{
    size_t index;
    float distance;
    size_t edges;
    Vector3f const * a;
    Vector3f const * b;
    SampleSegment(size_t const index_, Vector3f const& a_, Vector3f const& b_) : 
        index(index_), distance(a_.sq_dist_to(b_)), edges(1), a(&a_), b(&b_) {}
    bool operator<(SampleSegment const& other) const {
        return (distance / edges) < (other.distance / other.edges);
    }
    static bool index_compare(SampleSegment const& a, SampleSegment const& b) {
        // Should a go before b?
        return a.index < b.index;
    }
};

V3fList _upsample(V3fList const& points, size_t const target) {
    // Upsamples points to <target> number of point.
    DEBUG_NOMSG(points.size() > target);
    DEBUG_NOMSG(points.size() < 1);

    // Special case: single point upsampling = duplicating it <target> times.
    if (points.size() == 1) {
        V3fList result;
        for (size_t i = 0; i < target; ++i)
            result.push_back(points.at(0));
        return result;
    }

    // Construct segment max heap for selection.
    std::vector<SampleSegment> segments;
    for (size_t i = 1; i < points.size(); ++i)
        segments.emplace_back(i, points.at(i - 1), points.at(i));
    std::make_heap(segments.begin(), segments.end());

    // Insert a point in the centre of the longest segment until target is met.
    size_t remaining = target - points.size();
    while (remaining--) {
        // Remove longest segment (temporarily)
        std::pop_heap(segments.begin(), segments.end());

        // Split segment
        segments.back().edges++;

        // Put split segment back to heap
        std::push_heap(segments.begin(), std::prev(segments.end()));
    }

    // Pack result into vector
    std::sort_heap(segments.begin(), segments.end(), SampleSegment::index_compare);
    V3fList result = {points.at(0)};
    for (auto const& segment : segments) {
        // Insert <edges - 1> new points and point b
        Vector3f direction = (*segment.b) - (*segment.a);
        for (size_t i = 1; i < segment.edges; ++i) {
            result.push_back(direction * (static_cast<float>(i) / segment.edges));
        }
        result.push_back(*segment.b);
    }
    return result;
}

// These kabsch(mostly just SVD) functions taken from the Hybridization
// protocol of the Rosetta software suite, TMalign.cc. I split it into 2
// functions so there's no mode switching.
void _rosetta_kabsch_align(V3fList const& mobile,
                           V3fList const& ref,
                           elfin::Mat3f& rot,
                           Vector3f& tran)
{
    size_t const n = mobile.size();
    size_t const ref_n = ref.size();

    // Check sample sizes.
    DEBUG_NOMSG(n < 1);
    DEBUG_NOMSG(ref_n < 1);
    DEBUG_NOMSG(n != ref_n);

    int l = 0;
    int k = 0;
    double e0 = 0.0f;
    double d = 0.0f;
    double h = 0.0f;
    double g = 0.0f;
    double cth = 0.0f;
    double sth = 0.0f;
    double sqrth = 0.0f;
    double p = 0.0f;
    double det = 0.0f;
    double sigma = 0.0f;
    double xc[3] = { 0 }, yc[3] = { 0 };
    double a[3][3] = {0};
    double b[3][3] = {0};
    double r[3][3] = {0};
    double e[3] = {0};
    double rr[6] = {0};
    double ss[6] = {0};
    double sqrt3 = 1.73205080756888, tol = 0.01;
    int ip[] = {0, 1, 3, 1, 2, 4, 3, 4, 5};
    int ip2312[] = {1, 2, 0, 1};

    bool a_success = true, b_success = true;
    double epsilon = 0.00000001;

    // Compute centers for point vectors x, y.
    for (size_t i = 0; i < n; ++i) {
        xc[0] += mobile[i][0];
        xc[1] += mobile[i][1];
        xc[2] += mobile[i][2];

        yc[0] += ref[i][0];
        yc[1] += ref[i][1];
        yc[2] += ref[i][2];
    }

    for (size_t i = 0; i < 3; ++i) {
        xc[i] = xc[i] / n;
        yc[i] = yc[i] / n;
    }

    // Compute e0 and matrix r.
    for (size_t m = 0; m < n; m++) {
        for (size_t i = 0; i < 3; ++i) {
            e0 += (mobile[m][i] - xc[i]) * (mobile[m][i] - xc[i]) + \
                  (ref[m][i] - yc[i]) * (ref[m][i] - yc[i]);
            d = ref[m][i] - yc[i];
            for (size_t j = 0; j < 3; j++) {
                r[i][j] += d * (mobile[m][j] - xc[j]);
            }
        }
    }

    // Compute determinat of matrix r.
    det = r[0][0] * ( r[1][1] * r[2][2] - r[1][2] * r[2][1] )
          - r[0][1] * ( r[1][0] * r[2][2] - r[1][2] * r[2][0] )
          + r[0][2] * ( r[1][0] * r[2][1] - r[1][1] * r[2][0] );
    sigma = det;

    // Compute tras(r)*r.
    {
        size_t m = 0;
        for (size_t j = 0; j < 3; j++) {
            for (size_t i = 0; i <= j; ++i) {
                rr[m] = r[0][i] * r[0][j] + r[1][i] * r[1][j] + r[2][i] * r[2][j];
                m++;
            }
        }
    }

    double spur = (rr[0] + rr[2] + rr[5]) / 3.0;
    double cof = (((((rr[2] * rr[5] - rr[4] * rr[4]) + rr[0] * rr[5])
                    - rr[3] * rr[3]) + rr[0] * rr[2]) - rr[1] * rr[1]) / 3.0;
    det = det * det;

    for (size_t i = 0; i < 3; ++i) {
        e[i] = spur;
    }

    if (spur > 0) {
        d = spur * spur;
        h = d - cof;
        g = (spur * cof - det) / 2.0 - spur * h;

        if (h > 0) {
            sqrth = sqrt(h);
            d = h * h * h - g * g;
            if (d < 0.0 ) d = 0.0;
            d = atan2( sqrt(d), -g ) / 3.0;
            cth = sqrth * cos(d);
            sth = sqrth * sqrt3 * sin(d);
            e[0] = (spur + cth) + cth;
            e[1] = (spur - cth) + sth;
            e[2] = (spur - cth) - sth;

            for (l = 0; l < 3; l = l + 2) {
                d = e[l];
                ss[0] = (d - rr[2]) * (d - rr[5])  - rr[4] * rr[4];
                ss[1] = (d - rr[5]) * rr[1]      + rr[3] * rr[4];
                ss[2] = (d - rr[0]) * (d - rr[5])  - rr[3] * rr[3];
                ss[3] = (d - rr[2]) * rr[3]      + rr[1] * rr[4];
                ss[4] = (d - rr[0]) * rr[4]      + rr[1] * rr[3];
                ss[5] = (d - rr[0]) * (d - rr[2])  - rr[1] * rr[1];

                if (fabs(ss[0]) <= epsilon ) ss[0] = 0.0;
                if (fabs(ss[1]) <= epsilon ) ss[1] = 0.0;
                if (fabs(ss[2]) <= epsilon ) ss[2] = 0.0;
                if (fabs(ss[3]) <= epsilon ) ss[3] = 0.0;
                if (fabs(ss[4]) <= epsilon ) ss[4] = 0.0;
                if (fabs(ss[5]) <= epsilon ) ss[5] = 0.0;

                size_t jj;
                if (fabs(ss[0]) >= fabs(ss[2])) {
                    jj = 0;
                    if (fabs(ss[0]) < fabs(ss[5])) {
                        jj = 2;
                    }
                } else if (fabs(ss[2]) >= fabs(ss[5])) {
                    jj = 1;
                } else {
                    jj = 2;
                }

                d = 0.0;
                jj *= 3;
                for (size_t i = 0; i < 3; ++i) {
                    k = ip[i + jj];
                    a[i][l] = ss[k];
                    d = d + ss[k] * ss[k];
                }

                if (d > epsilon ) d = 1.0 / sqrt(d);
                else d = 0.0;
                for (size_t i = 0; i < 3; ++i) {
                    a[i][l] = a[i][l] * d;
                }
            }  // for l

            d = a[0][0] * a[0][2] + a[1][0] * a[1][2] + a[2][0] * a[2][2];

            size_t m1 = 0, m = 0;
            if ((e[0] - e[1]) > (e[1] - e[2])) {
                m1 = 2; m = 0;
            } else {
                m1 = 0; m = 2;
            }
            p = 0;
            for (size_t i = 0; i < 3; ++i) {
                a[i][m1] = a[i][m1] - d * a[i][m];
                p = p + a[i][m1] * a[i][m1];
            }
            if (p <= tol) {
                p = 1.0;
                size_t jj = 0;
                for (size_t i = 0; i < 3; ++i) {
                    if (p < fabs(a[i][m])) {
                        continue;
                    }
                    p = fabs( a[i][m] );
                    jj = i;
                }
                k = ip2312[jj];
                l = ip2312[jj + 1];
                p = sqrt( a[k][m] * a[k][m] + a[l][m] * a[l][m] );
                if (p > tol) {
                    a[jj][m1] = 0.0;
                    a[k][m1] = -a[l][m] / p;
                    a[l][m1] =  a[k][m] / p;
                } else {
                    a_success = false;
                }
            } else {
                p = 1.0 / sqrt(p);
                for (size_t i = 0; i < 3; ++i) {
                    a[i][m1] = a[i][m1] * p;
                }
            }
            if (a_success) {
                a[0][1] = a[1][2] * a[2][0] - a[1][0] * a[2][2];
                a[1][1] = a[2][2] * a[0][0] - a[2][0] * a[0][2];
                a[2][1] = a[0][2] * a[1][0] - a[0][0] * a[1][2];
            }

        }  //h>0

        if (a_success) {
            // Compute b.
            for (l = 0; l < 2; ++l) {
                d = 0.0;
                for (size_t i = 0; i < 3; ++i) {
                    b[i][l] = r[i][0] * a[0][l] + r[i][1] * a[1][l] + r[i][2] * a[2][l];
                    d = d + b[i][l] * b[i][l];
                }
                if (d > epsilon) {
                    d = 1.0 / sqrt(d);
                } else {
                    d = 0.0;
                }
                for (size_t i = 0; i < 3; ++i) {
                    b[i][l] = b[i][l] * d;
                }
            }
            d = b[0][0] * b[0][1] + b[1][0] * b[1][1] + b[2][0] * b[2][1];
            p = 0.0;

            for (size_t i = 0; i < 3; ++i) {
                b[i][1] = b[i][1] - d * b[i][0];
                p += b[i][1] * b[i][1];
            }

            if (p <= tol) {
                p = 1.0;
                size_t jj = 0;
                for (size_t i = 0; i < 3; ++i) {
                    if (p < fabs(b[i][0])) {
                        continue;
                    }
                    p = fabs( b[i][0] );
                    jj = i;
                }
                k = ip2312[jj];
                l = ip2312[jj + 1];
                p = sqrt( b[k][0] * b[k][0] + b[l][0] * b[l][0] );
                if (p > tol) {
                    b[jj][1] = 0.0;
                    b[k][1] = -b[l][0] / p;
                    b[l][1] =  b[k][0] / p;
                } else {
                    b_success = false;
                }
            } else {
                p = 1.0 / sqrt(p);
                for (size_t i = 0; i < 3; ++i) {
                    b[i][1] = b[i][1] * p;
                }
            }
            if (b_success) {
                b[0][2] = b[1][0] * b[2][1] - b[1][1] * b[2][0];
                b[1][2] = b[2][0] * b[0][1] - b[2][1] * b[0][0];
                b[2][2] = b[0][0] * b[1][1] - b[0][1] * b[1][0];
            }
        }  // if(a_success)
    }

    // Compute rot.
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; j++) {
            rot[i][j] = b[i][0] * a[j][0] +
                        b[i][1] * a[j][1] +
                        b[i][2] * a[j][2];
        }
    }

    // Compute t.
    for (size_t i = 0; i < 3; ++i) {
        tran[i] = ((yc[i] - rot[i][0] * xc[0]) - rot[i][1] * xc[1]) -
                  rot[i][2] * xc[2];
    }
}

}  /* kabsch */

}  /* elfin */