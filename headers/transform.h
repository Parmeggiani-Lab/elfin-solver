#ifndef TRANSFOREIGEN_H_
#define TRANSFOREIGEN_H_

// #define USE_EIGEN_IMPL

#include <sstream>

#ifdef USE_EIGEN_IMPL
#include "eigen.h"
#endif  /* ifdef USE_EIGEN_IMPL */


#include "json.h"


namespace elfin {

class Vector3f;

#ifdef USE_EIGEN_IMPL
using Matrix4f = Eigen::Matrix4f;
#define TX_INHERIT : public Matrix4f
#else
#define TX_INHERIT
#endif  /* ifdef USE_EIGEN_IMPL */

class Transform TX_INHERIT {
private:

#ifndef USE_EIGEN_IMPL

    /* data */
    float rot_[3][3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    float tran_[3] = {0, 0, 0};

#endif  /* ifndef USE_EIGEN_IMPL */

public:

    /* ctors */
#ifdef USE_EIGEN_IMPL

    // Construct Transform from Eigen expressions
    template<typename OtherDerived>
    Transform(
        Eigen::MatrixBase<OtherDerived> const& other)
        : Matrix4f(other) {}
    Transform() : Matrix4f(Identity()) {}

#else

    Transform() {}

#endif  /* ifdef USE_EIGEN_IMPL */

    Transform(JSON const& tx_json);

    /* accessors */
    Vector3f collapsed() const;
    Transform inversed() const;

#ifdef USE_EIGEN_IMPL
    template<typename OtherDerived>
    Transform operator*(
        Eigen::MatrixBase<OtherDerived> const & rhs) const {
        return Matrix4f::operator*(rhs);
    }
#else
    Transform operator*(Transform const & rhs) const;
#endif  /* ifdef USE_EIGEN_IMPL */

    Vector3f operator*(Vector3f const & vec) const;
    bool is_approx(Transform const& other) const;

    /* modifiers */
#ifdef USE_EIGEN_IMPL
    // Assign Eigen expressions to Transform
    template<typename OtherDerived>
    Transform & operator=(
        Eigen::MatrixBase<OtherDerived> const& other)
    {
        this->Matrix4f::operator=(other);
        return *this;
    }
#endif  /* ifdef USE_EIGEN_IMPL */

    /* printers */
#ifndef USE_EIGEN_IMPL

    std::string to_string() const {
        std::ostringstream ss;

        ss << "Transform[\nRot[";
        for (size_t i = 0; i < 3; ++i) {
            ss << "  [ ";
            for (size_t j = 0; j < 3; ++j) {
                ss << rot_[i][j];
                if (j < 3) {
                    ss << ", ";
                }
            }
            ss << " ]\n";
        }
        ss << "]\n";

        ss << "Tran[" << tran_[0];
        ss << ", " << tran_[1];
        ss << ", " << tran_[2] << "]";

        return ss.str();
    }

#endif  /* ifndef USE_EIGEN_IMPL */

    /* tests */
    static void test(size_t& errors, size_t& tests);
};

}  /* elfin */

#undef TX_IMPL

#endif  /* end of include guard: TRANSFORM_EIGEN_IMPL_H_ */