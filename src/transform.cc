#include "transform.h"

#include "debug_utils.h"
#include "vector3f.h"

namespace elfin {

/* ctors */
Transform::Transform(JSON const& tx_json) {
    JSON const& rot = tx_json["rot"];
    NICE_PANIC(rot.size() != 3);
    NICE_PANIC(rot[0].size() != 3);

    JSON const& tran = tx_json["tran"];
    NICE_PANIC(tran.size() != 3);

    (*this) << rot[0][0], rot[0][1], rot[0][2], tran[0],
    rot[1][0], rot[1][1], rot[1][2], tran[1],
    rot[2][0], rot[2][1], rot[2][2], tran[2],
    0.f, 0.f, 0.f, 1.f;
}

/* accessors */
Vector3f Transform::collapsed() const {
    return block<3, 1>(0, 3);
}

Transform Transform::inversed() const {
    Eigen::Matrix3f const inv_rot =
        block<3, 3>(0, 0).transpose();
    Eigen::Vector3f const inv_tran =
        inv_rot * -block<3, 1>(0, 3);
    Transform res;
    res << inv_rot(0, 0), inv_rot(0, 1), inv_rot(0, 2), inv_tran(0),
        inv_rot(1, 0), inv_rot(1, 1), inv_rot(1, 2), inv_tran(1),
        inv_rot(2, 0), inv_rot(2, 1), inv_rot(2, 2), inv_tran(2),
        0, 0, 0, 1;
    return res;
}

Transform Transform::operator*(Transform const& rhs) const {
    return EigenTransform::operator*(rhs);
}

Vector3f Transform::operator*(
    Vector3f const& vec) const {
    return block<3, 3>(0, 0) * vec + block<3, 1>(0, 3);
}

/* tests */
void Transform::test(size_t& errors, size_t& tests) {
    UNIMPLEMENTED();
}

}  /* elfin */