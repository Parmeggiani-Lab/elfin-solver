#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "eigen.h"
#include "json.h"

namespace elfin {

class Vector3f;

typedef Eigen::AffineCompact3f EigenTransform;
class Transform : public EigenTransform {
public:
    /* ctors */
    using EigenTransform::EigenTransform;

    // Construct Transform from Eigen expressions
    template<typename OtherDerived>
    Transform(
        Eigen::EigenBase<OtherDerived> const& other)
        : EigenTransform(other) {}
    Transform() : EigenTransform(Identity()) {}
    Transform(JSON const& tx_json);

    /* accessors */
    Vector3f collapsed() const;

    /* modifiers */
    // Assign Eigen expressions to Transform
    template<typename OtherDerived>
    Transform& operator=(
        Eigen::EigenBase<OtherDerived> const& other)
    {
        this->EigenTransform::operator=(other);
        return *this;
    }

    /* tests */
    static void test(size_t& errors, size_t& tests);
};

// class Transform {
// private:
//     /* types */
//     typedef Eigen::Matrix<float, 3, 4> TxMat;

//     /* data */
//     TxMat tx_;

// public:
//     /* ctors */
//     Transform() : tx_(TxMat::Identity()) {}
//     Transform(Transform const& other) {
//         *this = other; // call operator=(const T&)
//     }
//     // Transform(Transform && other);
//     Transform(JSON const& tx_json);

//     /* accessors */
//     Transform operator*(Transform const& rhs) const;
//     Vector3f operator*(Vector3f const& vec) const;
//     Transform inversed() const;
//     Vector3f collapsed() const;
//     bool operator==(Transform const& other) const {
//         return tx_.isApprox(other.tx_);
//     }
//     bool operator!=(Transform const& other) const {
//         return not this->operator==(other);
//     }

//     /* modifiers */
//     Transform& operator=(Transform const& other) {
//         if (this != &other) {
//             tx_ = other.tx_;
//         }
//         return *this;
//     }
//     // Transform& operator=(Transform && other);
//     // Transform& operator*=(Transform const& rhs);

//     /* printers */
//     std::string to_string() const;
//     std::string to_csv_string() const;

//     /* tests */
//     static void test(size_t& errors, size_t& tests);
// };

}  /* elfin */

#endif  /* end of include guard: TRANSFORM_H_ */