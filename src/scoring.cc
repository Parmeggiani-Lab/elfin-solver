#include <cmath>

#include "scoring.h"

#include "work_area.h"
#include "debug_utils.h"
#include "test_data.h"

namespace elfin {

namespace scoring {

void calc_alignment(
    V3fList const& mobile,
    V3fList const& ref,
    elfin::Mat3f& rot,
    Vector3f& tran,
    float& rms) {
    V3fList const& mobile_resampled =
        mobile.size() == ref.size() ?
        mobile : _resample(ref, mobile);

    _rosetta_kabsch_align(mobile_resampled, ref, rot, tran, rms);
}

float score(V3fList const& mobile, V3fList const& ref) {
    if (mobile.empty() or ref.empty()) {
        return INFINITY;
    }

    V3fList const& mobile_resampled =
        mobile.size() == ref.size() ?
        mobile : _resample(ref, mobile);

    return _rosetta_kabsch_rms(mobile_resampled, ref);
}

float simple_rms(V3fList const& mobile, V3fList const& ref) {
    if (mobile.empty() or ref.empty()) {
        return INFINITY;
    }

    V3fList const& mobile_resampled =
        mobile.size() == ref.size() ?
        mobile : _resample(ref, mobile);

    size_t const n = ref.size();
    double sq_err = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        sq_err += ref.at(i).sq_dist_to(mobile_resampled.at(i));
    }

    return sqrt(sq_err / n);
}

V3fList _resample(
    V3fList const& ref,
    V3fList const& pts) {
    if (ref.size() == pts.size())
        return pts;

    size_t const N = ref.size();

    // Compute shape total lengths.
    float ref_tot_len = 0.0f;
    for (size_t i = 1; i < N; ++i)
        ref_tot_len += ref.at(i).dist_to(ref.at(i - 1));

    float pts_tot_len = 0.0f;
    for (size_t i = 1; i < pts.size(); ++i)
        pts_tot_len += pts.at(i).dist_to(pts.at(i - 1));

    // Upsample points.
    V3fList resampled;

    // First and last points are the same.
    resampled.push_back(pts.at(0));

    float ref_prop = 0.0f, pts_prop = 0.0f;
    int mpi = 1;
    for (size_t i = 1; i < pts.size(); ++i) {
        Vector3f const& base_fp_point = pts.at(i - 1);
        Vector3f const& next_fp_point = pts.at(i);
        float const base_fp_proportion = pts_prop;
        float const fp_segment = next_fp_point.dist_to(base_fp_point)
                                 / pts_tot_len;
        Vector3f const vec = next_fp_point - base_fp_point;

        pts_prop += fp_segment;
        while (ref_prop <= pts_prop && mpi < N) {
            float const mpSegment =
                ref.at(mpi).dist_to(ref.at(mpi - 1))
                / ref_tot_len;

            if (ref_prop + mpSegment > pts_prop)
                break;
            ref_prop += mpSegment;

            float const s = (ref_prop - base_fp_proportion)
                            / fp_segment;
            resampled.push_back(base_fp_point + (vec * s));

            mpi++;
        }
    }

    // Sometimes the last node is automatically added
    if (resampled.size() < N) {
        resampled.push_back(pts.back());
    }

    // Is something wrong with the resampling algorithm?
    TRACE_NOMSG(resampled.size() != ref.size());

    return resampled;
}

// These kabsch(mostly just SVD) functions taken from the Hybridization
// protocol of the Rosetta software suite, TMalign.cc. I split it into 2
// functions so there's no mode switching.
void _rosetta_kabsch_align(
    V3fList const& mobile,
    V3fList const& ref,
    elfin::Mat3f& rot,
    Vector3f& tran,
    float& rms)
{
    size_t const n = mobile.size();
    size_t const ref_n = ref.size();

    // Check sample sizes.
    DEBUG_NOMSG(n < 1);
    DEBUG_NOMSG(ref_n < 1);
    DEBUG_NOMSG(n != ref_n);

    size_t j = 0;
    size_t m = 0;
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

    // Compute centers for vector sets x, y
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
    for (m = 0; m < n; m++) {
        for (size_t i = 0; i < 3; ++i) {
            e0 += (mobile[m][i] - xc[i]) * (mobile[m][i] - xc[i]) + \
                  (ref[m][i] - yc[i]) * (ref[m][i] - yc[i]);
            d = ref[m][i] - yc[i];
            for (j = 0; j < 3; j++) {
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
    m = 0;
    for (j = 0; j < 3; j++) {
        for (size_t i = 0; i <= j; ++i) {
            rr[m] = r[0][i] * r[0][j] + r[1][i] * r[1][j] + r[2][i] * r[2][j];
            m++;
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

                if (fabs(ss[0]) >= fabs(ss[2])) {
                    j = 0;
                    if (fabs(ss[0]) < fabs(ss[5])) {
                        j = 2;
                    }
                } else if (fabs(ss[2]) >= fabs(ss[5])) {
                    j = 1;
                } else {
                    j = 2;
                }

                d = 0.0;
                j = 3 * j;
                for (size_t i = 0; i < 3; ++i) {
                    k = ip[i + j];
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
                for (size_t i = 0; i < 3; ++i) {
                    if (p < fabs(a[i][m])) {
                        continue;
                    }
                    p = fabs( a[i][m] );
                    j = i;
                }
                k = ip2312[j];
                l = ip2312[j + 1];
                p = sqrt( a[k][m] * a[k][m] + a[l][m] * a[l][m] );
                if (p > tol) {
                    a[j][m1] = 0.0;
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
                for (size_t i = 0; i < 3; ++i) {
                    if (p < fabs(b[i][0])) {
                        continue;
                    }
                    p = fabs( b[i][0] );
                    j = i;
                }
                k = ip2312[j];
                l = ip2312[j + 1];
                p = sqrt( b[k][0] * b[k][0] + b[l][0] * b[l][0] );
                if (p > tol) {
                    b[j][1] = 0.0;
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
        for (j = 0; j < 3; j++) {
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

float _rosetta_kabsch_rms(
    V3fList const& mobile,
    V3fList const& ref)
{
    size_t const n = mobile.size();
    size_t const ref_n = ref.size();

    // Check sample sizes.
    DEBUG_NOMSG(n < 1);
    DEBUG_NOMSG(ref_n < 1);
    DEBUG_NOMSG(n != ref_n);

    size_t j = 0;
    size_t m = 0;
    int l = 0;
    int k = 0;
    double e0 = 0.0f;
    double d = 0.0f;
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

    bool a_success = true, b_success = true;

    // Compute centers for vector sets x, y
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
    for (m = 0; m < n; m++) {
        for (size_t i = 0; i < 3; ++i) {
            e0 += (mobile[m][i] - xc[i]) * (mobile[m][i] - xc[i]) + \
                  (ref[m][i] - yc[i]) * (ref[m][i] - yc[i]);
            d = ref[m][i] - yc[i];
            for (j = 0; j < 3; j++) {
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
    m = 0;
    for (j = 0; j < 3; j++) {
        for (size_t i = 0; i <= j; ++i) {
            rr[m] = r[0][i] * r[0][j] + r[1][i] * r[1][j] + r[2][i] * r[2][j];
            m++;
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
        }  //h>0
    }

    // Compute rms.
    for (size_t i = 0; i < 3; ++i) {
        if (e[i] < 0 ) e[i] = 0;
        e[i] = sqrt( e[i] );
    }

    d = e[2];

    if (sigma < 0.0) {
        d = - d;
    }

    d = (d + e[1]) + e[0];
    float rms = (e0 - d) - d;

    if (rms < 0.0 ) {
        rms = 0.0;
    }

    return rms;
}

}  /* kabsch */

}  /* elfin */