#include "transform.h"

#include <cmath>

#include "debug_utils.h"
#include "vector3f.h"

namespace elfin {

/* ctors */
Transform::Transform(JSON const& tx_json) {
    JSON const& rot_json = tx_json["rot"];
    NICE_PANIC(rot_json.size() != 3);
    NICE_PANIC(rot_json[0].size() != 3);

    JSON const& tran_json = tx_json["tran"];
    NICE_PANIC(tran_json.size() != 3);

#ifdef USE_EIGEN
    (*this) << rot_json[0][0], rot_json[0][1], rot_json[0][2], tran_json[0],
    rot_json[1][0], rot_json[1][1], rot_json[1][2], tran_json[1],
    rot_json[2][0], rot_json[2][1], rot_json[2][2], tran_json[2],
    0.f, 0.f, 0.f, 1.f;
#else
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            rot_[i][j] = rot_json[i][j];
        }
        tran_[i] = tran_json[i];
    }
#endif  /* ifdef USE_EIGEN */

}

/* accessors */
Vector3f Transform::collapsed() const {
#ifdef USE_EIGEN
    return block<3, 1>(0, 3);
#else
    return Vector3f(tran_[0], tran_[1], tran_[2]);
#endif  /* ifdef USE_EIGEN */
}

Transform Transform::inversed() const {

#ifdef USE_EIGEN
    // res.block<3, 3>(0, 0) = block<3, 3>(0, 0).transpose();
    // res.block<3, 1>(0, 3) = -(res.block<3, 3>(0, 0) * block<3, 1>(0, 3));
    return inverse();
#else
    Transform res;

    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            res.rot_[i][j] = rot_[j][i];
        }
    }

    res.tran_[0] = -(res.rot_[0][0] * tran_[0] +
                     res.rot_[0][1] * tran_[1] +
                     res.rot_[0][2] * tran_[2]);

    res.tran_[1] = -(res.rot_[1][0] * tran_[0] +
                     res.rot_[1][1] * tran_[1] +
                     res.rot_[1][2] * tran_[2]);

    res.tran_[2] = -(res.rot_[2][0] * tran_[0] +
                     res.rot_[2][1] * tran_[1] +
                     res.rot_[2][2] * tran_[2]);

    return res;
#endif  /* ifdef USE_EIGEN */
}

#ifndef USE_EIGEN

Transform Transform::operator*(Transform const& rhs) const {
    Transform res;

    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            res.rot_[i][j] =
                rot_[i][0] * rhs.rot_[0][j] +
                rot_[i][1] * rhs.rot_[1][j] +
                rot_[i][2] * rhs.rot_[2][j];
        }
        res.tran_[i] =
            rot_[i][0] * rhs.tran_[0] +
            rot_[i][1] * rhs.tran_[1] +
            rot_[i][2] * rhs.tran_[2] + tran_[i] ;
    }

    return res;
}

#endif  /* ifndef USE_EIGEN */

bool Transform::is_approx(
    Transform const& other,
    float const tolerance) const {
#ifdef USE_EIGEN
    return isApprox(other);
#else
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (not float_approximates_err(
                        rot_[i][j],
                        other.rot_[i][j],
                        tolerance)) {
                return false;
            }
        }

        if (not float_approximates_err(
                    tran_[i],
                    other.tran_[i],
                    tolerance)) {
            return false;
        }
    }

    return true;
#endif  /* ifdef USE_EIGEN */
}

/* tests */
TestStat Transform::test() {
    TestStat ts;

    /* Frame Shift Test */
    JSON a_world_json = {
        {   "rot", {
                {0.05532475933432579, 0.017428744584321976, -0.08145802468061447},
                { -0.049986086785793304, 0.08517082780599594, -0.015726475045084953},
                {0.0666375458240509, 0.049418311566114426, 0.055832501500844955}
            }
        },
        {"tran", {8.054840087890625, 1.978692889213562, 3.4368598461151123}}
    };

    JSON a_to_b_json = {
        {   "rot", {
                {0.6502372026443481, 0.3104279935359955, 0.6934162378311157},
                {0.756941020488739, -0.3428647220134735, -0.5563129186630249},
                {0.06505285948514938, 0.8866105675697327, -0.45791906118392944}
            }
        },
        {"tran", { -12.447150230407715, -10.713072776794434, -29.79046058654785}}
    };

    JSON b_world_json = {
        {   "rot", {
                { -0.015099741518497467, 0.08121803402900696, 0.05635272338986397},
                { -0.016968388110399246, -0.058289747685194016, 0.07946307212114334},
                {0.0973862037062645, 0.0024365708231925964, 0.022582994773983955}
            }
        },
        {"tran", {10.415760040283203, 3.510263681411743, 5.347901344299316}}
    };

    JSON c_to_a_json = {
        {   "rot", {
                {0.555465579032898, 0.8273841142654419, -0.08302728086709976},
                { -0.08186252415180206, 0.1537732481956482, 0.9847092628479004},
                {0.8275001645088196, -0.5401752591133118, 0.15314750373363495}
            }
        },
        {"tran", { -11.876138687133789, -42.8249397277832, 10.272198677062988}}
    };

    JSON c_world_json = {
        {   "rot", {
                { -0.03810230642557144, 0.09245651960372925, 9.370781481266022e-05},
                { -0.047751497477293015, -0.01976567879319191, 0.08561023324728012},
                {0.07917074859142303, 0.03257473558187485, 0.0516805462539196}
            }
        },
        {"tran", {5.814658164978027, -1.2366465330123901, 1.102649211883545}}
    };

    Transform a_world(a_world_json);
    Transform a_to_b(a_to_b_json);
    Transform b_world(b_world_json);
    Transform c_to_a(c_to_a_json);
    Transform c_world(c_world_json);

    Transform b_test = a_world * a_to_b.inversed(); // C-term extrude "raise"
    ts.tests++;
    if (not b_test.is_approx(b_world)) {
        ts.errors++;
        err("Eigen frame shift test failed:\n"
            "b_test does not approximately equal to b_world\n");
        err("b_test: %s\n", b_test.to_string().c_str());
        err("b_world: %s\n", b_world.to_string().c_str());
    }

    Transform c_test = a_world * c_to_a; // N-term extrude "drop"
    ts.tests++;
    if (not c_test.is_approx(c_world)) {
        ts.errors++;
        err("Eigen frame shift test failed:\n"
            "c_test does not approximately equal to c_world\n");
        err("c_test: %s\n", c_test.to_string().c_str());
        err("c_world: %s\n", c_world.to_string().c_str());
    }

    return ts;
}

}  /* elfin */