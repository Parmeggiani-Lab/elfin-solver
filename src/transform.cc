#include "transform.h"

#include <cmath>

#include "debug_utils.h"
#include "vector3f.h"
#include "test_consts.h"

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

Transform::Transform(Vector3f const& vec) {
#ifdef USE_EIGEN
    (*this) << 0.f, 0.f, 0.f, vec[0],
    0.f, 0.f, 0.f, vec[1],
    0.f, 0.f, 0.f, vec[2],
    0.f, 0.f, 0.f, 1.f;
#else
    for (size_t i = 0; i < 3; ++i) {
        tran_[i] = vec[i];
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

Vector3f Transform::operator*(Vector3f const& rhs) const {
    return operator*(Transform(rhs)).collapsed();
}

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

    // Frame Shift Tests

    // Test C-term extrude "raise".
    {
        Transform b_test = a_world * a_to_b.inversed();
        ts.tests++;
        if (not b_test.is_approx(b_world)) {
            ts.errors++;
            err("Frame raise test failed:\n"
                "b_test does not approximately equal to b_world\n");
            err("Expected b_world: %s\n", b_world.to_string().c_str());
            err("Got b_test: %s\n", b_test.to_string().c_str());
        }
    }

    // Test N-term extrude "drop".
    {
        Transform c_test = a_world * c_to_a;
        ts.tests++;
        if (not c_test.is_approx(c_world)) {
            ts.errors++;
            err("Frame drop test failed:\n"
                "c_test does not approximately equal to c_world\n");
            err("Expected c_world: %s\n", c_world.to_string().c_str());
            err("Got c_test: %s\n", c_test.to_string().c_str());
        }
    }

    return ts;
}

}  /* elfin */