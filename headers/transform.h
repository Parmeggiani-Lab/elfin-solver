#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "eigen.h"
#include "json.h"

namespace elfin {

class Vector3f;

typedef Eigen::Matrix4f EigenTransform;
class Transform : public EigenTransform {
public:
    /* ctors */

    // Construct Transform from Eigen expressions
    template<typename OtherDerived>
    Transform(
        Eigen::MatrixBase<OtherDerived> const& other)
        : EigenTransform(other) {}
    Transform() : EigenTransform(Identity()) {}
    Transform(JSON const& tx_json);

    /* accessors */
    Vector3f collapsed() const;
    Transform inversed() const;
    Transform operator*(Transform const& rhs) const;
    Vector3f operator*(Vector3f const& vec) const;

    /* modifiers */
    // Assign Eigen expressions to Transform
    template<typename OtherDerived>
    Transform& operator=(
        Eigen::MatrixBase<OtherDerived> const& other)
    {
        this->EigenTransform::operator=(other);
        return *this;
    }

    /* tests */
    static void test(size_t& errors, size_t& tests);
};

}  /* elfin */

#endif  /* end of include guard: TRANSFORM_H_ */