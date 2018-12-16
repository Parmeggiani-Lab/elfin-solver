#include "transform.h"

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

#ifdef USE_EIGEN_IMPL

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

#endif  /* ifdef USE_EIGEN_IMPL */

}

/* accessors */
Vector3f Transform::collapsed() const {
#ifdef USE_EIGEN_IMPL

    return block<3, 1>(0, 3);

#else

    return Vector3f(tran_[0], tran_[1], tran_[2]);

#endif  /* ifdef USE_EIGEN_IMPL */
}

Transform Transform::inversed() const {
#ifdef USE_EIGEN_IMPL

    Transform res;

    res.block<3, 3>(0, 0) = block<3, 3>(0, 0).transpose();
    res.block<3, 1>(0, 3) = -(res.block<3, 3>(0, 0) * block<3, 1>(0, 3));

    return res;

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

#endif  /* ifdef USE_EIGEN_IMPL */
}

#ifndef USE_EIGEN_IMPL

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

#endif  /* ifndef USE_EIGEN_IMPL */

Vector3f Transform::operator*(
    Vector3f const& vec) const {
#ifdef USE_EIGEN_IMPL

    return block<3, 3>(0, 0) * vec + block<3, 1>(0, 3);

#else

    float const rx = rot_[0][0] * vec[0] +
                     rot_[0][1] * vec[1] +
                     rot_[0][2] * vec[2] +
                     tran_[0];

    float const ry = rot_[1][0] * vec[0] +
                     rot_[1][1] * vec[1] +
                     rot_[1][2] * vec[2] +
                     tran_[1];

    float const rz = rot_[2][0] * vec[0] +
                     rot_[2][1] * vec[1] +
                     rot_[2][2] * vec[2] +
                     tran_[2];

    return Vector3f(rx, ry, rz);

#endif  /* ifdef USE_EIGEN_IMPL */
}

bool Transform::is_approx(Transform const& other) const {
#ifdef USE_EIGEN_IMPL

    return isApprox(other);

#else

    /*
     * We use 0.0001 as tolerance here because that's the highest precision
     * PDBs support. 
     */
    float const tolerance = 1e-4;

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

#endif  /* ifdef USE_EIGEN_IMPL */
}

/* tests */
void Transform::test(size_t& errors, size_t& tests) {
    UNIMPLEMENTED();
}

}  /* elfin */