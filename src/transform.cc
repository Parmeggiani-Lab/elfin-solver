#include "transform.h"

#include "debug_utils.h"
#include "vector3f.h"

namespace elfin {

/* ctors */
Transform::Transform(JSON const& tx_json) {
    MatrixType & matrix = this->matrix();

    JSON const& rot = tx_json["rot"];
    NICE_PANIC(rot.size() != 3);
    NICE_PANIC(rot[0].size() != 3);

    // rot << rot[0][0], rot[0][1], rot[0][2],
    //     rot[1][0], rot[1][1], rot[1][2],
    //     rot[2][0], rot[2][1], rot[2][2];

    JSON const& tran = tx_json["tran"];
    NICE_PANIC(tran.size() != 3);

    // tran << tran[0], tran[1], tran[2];

    matrix << rot[0][0], rot[0][1], rot[0][2], tran[0],
           rot[1][0], rot[1][1], rot[1][2], tran[1],
           rot[2][0], rot[2][1], rot[2][2], tran[2];
}

/* accessors */
Vector3f Transform::collapsed() const {
    return this->operator*(Vector3f());
    // return rot * TranVec::Identity() + tran;
}

/* tests */
void Transform::test(size_t& errors, size_t& tests) {
    UNIMPLEMENTED();
}

}  /* elfin */