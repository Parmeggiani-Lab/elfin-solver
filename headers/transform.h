#ifndef TRANSFOREIGEN_H_
#define TRANSFOREIGEN_H_

// #define USE_EIGEN

#include <sstream>

#ifdef USE_EIGEN
#include "eigen.h"
#endif  /* ifdef USE_EIGEN */

#include "json.h"
#include "string_utils.h"

namespace elfin {

class Vector3f;
typedef Vector3f Mat3f[3];
struct TestStat;

#ifdef USE_EIGEN
using Matrix4f = Eigen::Matrix4f;
#define TX_INHERIT : public Matrix4f, public Printable
#else
#define TX_INHERIT : public Printable
#endif  /* ifdef USE_EIGEN */

class Transform TX_INHERIT {
#undef TX_INHERIT

private:
#ifndef USE_EIGEN
    /* data */
    float rot_[3][3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    float tran_[3] = {0, 0, 0};
#endif  /* ifndef USE_EIGEN */

public:

    /* ctors */
#ifdef USE_EIGEN
    // Construct Transform from Eigen expressions
    template<typename OtherDerived>
    Transform(
        Eigen::MatrixBase<OtherDerived> const& other)
        : Matrix4f(other) {}
    Transform() : Matrix4f(Identity()) {}
#else
    Transform() {}
#endif  /* ifdef USE_EIGEN */

    Transform(JSON const& tx_json);
    Transform(Vector3f const& vec);
    Transform(elfin::Mat3f const& rot, Vector3f const& vec);

    /* accessors */
    Vector3f collapsed() const;
    Transform inversed() const;

#ifdef USE_EIGEN
    template<typename OtherDerived>
    Transform operator*(
        Eigen::MatrixBase<OtherDerived> const& rhs) const {
        return Matrix4f::operator*(rhs);
    }
#else
    Transform operator*(Transform const& rhs) const;
#endif  /* ifdef USE_EIGEN */

    Vector3f operator*(Vector3f const& rhs) const;

    /*
     * We use 0.0001 as tolerance here because that's the highest precision
     * PDBs support.
     */
    bool is_approx(Transform const& other,
                   float const tolerance = 1e-4) const;

    JSON rot_json() const;
    JSON tran_json() const;

    /* modifiers */
#ifdef USE_EIGEN
    // Assign Eigen expressions to Transform
    template<typename OtherDerived>
    Transform & operator=(
        Eigen::MatrixBase<OtherDerived> const& other) {
        this->Matrix4f::operator=(other);
        return *this;
    }
#endif  /* ifdef USE_EIGEN */

    /* printers */
    virtual void print_to(std::ostream& os) const;

    /* tests */
    static TestStat test();
};

}  /* elfin */

#undef TX_IMPL

#endif  /* end of include guard: TRANSFORM_EIGEN_IMPL_H_ */