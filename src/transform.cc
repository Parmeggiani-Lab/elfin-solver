#include "transform.h"

#include <cmath>
#include <vector>

#include "debug_utils.h"
#include "vector3f.h"

namespace elfin {

/* ctors */
Transform::Transform(JSON const& tx_json) {
    JSON const& rot_json = tx_json["rot"];
    TRACE_NOMSG(rot_json.size() != 3);
    TRACE_NOMSG(rot_json[0].size() != 3);

    JSON const& tran_json = tx_json["tran"];
    TRACE_NOMSG(tran_json.size() != 3);
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

Transform::Transform(elfin::Mat3f const& rot, Vector3f const& vec) {
#ifdef USE_EIGEN
    (*this) << rot[0][0], rot[0][1], rot[0][2], vec[0],
    rot[1][0], rot[1][1], rot[1][2], vec[1],
    rot[2][0], rot[2][1], rot[2][2], vec[2],
    0.f, 0.f, 0.f, 1.f;
#else
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            rot_[i][j] = rot[i][j];
        }
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
            if (not JUtil.float_approximates(
                        rot_[i][j],
                        other.rot_[i][j],
                        tolerance)) {
                return false;
            }
        }

        if (not JUtil.float_approximates(
                    tran_[i],
                    other.tran_[i],
                    tolerance)) {
            return false;
        }
    }

    return true;
#endif  /* ifdef USE_EIGEN */
}

JSON Transform::rot_json() const {
    JSON out;
#ifdef USE_EIGEN
    out = {

    };
#else
    for (size_t i = 0; i < 3; ++i) {
        out[i] = std::vector<float>(rot_[i], rot_[i] + 3);
    }
#endif  /* ifdef USE_EIGEN */
    return out;
}

JSON Transform::tran_json() const {
    JSON out;
#ifdef USE_EIGEN
#else
    out = std::vector<float>(tran_, tran_ + 3);
#endif  /* ifdef USE_EIGEN */
    return out;
}

/* printers */
#ifdef USE_EIGEN
Eigen::IOFormat const CleanFormt =
    Eigen::IOFormat(4, 0, ", ", "\n", "[", "]");
#endif  /* ifdef USE_EIGEN */

void Transform::print_to(std::ostream& os) const {
#ifdef USE_EIGEN
    os << "Transform[\n" << this->format(CleanFormt) << "\n]";
#else
    os << "Transform[\n  Rot[\n";
    for (size_t i = 0; i < 3; ++i) {
        os << "    [ ";
        for (size_t j = 0; j < 3; ++j) {
            os << string_format("%.4f", rot_[i][j]);
            if (j < 2) {
                os << ", ";
            }
        }
        os << " ]\n";
    }
    os << "  ]\n";

    os << "  Tran[ " << tran_[0];
    os << ", " << tran_[1];
    os << ", " << tran_[2] << " ]\n";
    os << "]";
#endif  /* ifdef USE_EIGEN */
}

}  /* elfin */