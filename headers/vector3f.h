#ifndef VECTOR3F_H_
#define VECTOR3F_H_

// #define USE_EIGEN
#ifdef USE_EIGEN
#define V3F_USE_EIGEN
#endif /* ifdef USE_EIGEN */

#include <vector>

#ifdef V3F_USE_EIGEN
#include "eigen.h"
#else
#include <cmath>
#endif  /* ifdef V3F_USE_EIGEN */

#include "debug_utils.h"

namespace elfin {



struct TestStat;

#ifdef V3F_USE_EIGEN
typedef Eigen::Vector3f EigenV3f;
#define V3F_INHERIT : public EigenV3f
#else
#define V3F_INHERIT
#endif  /* ifdef V3F_USE_EIGEN */



class Vector3f V3F_INHERIT {
private:

#ifndef V3F_USE_EIGEN
    /* data */
    float data_[3] = { 0, 0, 0 };
#endif  /* ifdef V3F_USE_EIGEN */

    /* ctors */
    template<class RandomAccessIterator >
    Vector3f(
        RandomAccessIterator begin,
        RandomAccessIterator end) {
        NICE_PANIC((end - begin) < 3,
                   string_format(
                       "Invalid Argument Size: %lu, should be <3\n",
                       end - begin));
        auto itr = begin;

#ifdef V3F_USE_EIGEN
        Vector3f& data_ = *this;
#endif  /* ifdef V3F_USE_EIGEN */

        data_[0] = *itr++;
        data_[1] = *itr++;
        data_[2] = *itr++;
    }

public:
    /* ctors */


#ifdef V3F_USE_EIGEN
    // Construct Vector3f from Eigen expressions
    template<typename OtherDerived>
    Vector3f(
        Eigen::MatrixBase<OtherDerived> const& other)
        : EigenV3f(other) {}
    Vector3f() : EigenV3f(EigenV3f::Zero()) {}
    Vector3f(float x, float y, float z) : EigenV3f(x, y, z) {}
#else
    Vector3f() {}
    Vector3f(float x, float y, float z) {
        data_[0] = x, data_[1] = y, data_[2] = z;
    }
#endif  /* ifdef V3F_USE_EIGEN */

    template <typename T>
    Vector3f(std::vector<T> const& vec) :
        Vector3f(vec.cbegin(), vec.cbegin() + 3) {}

    /* accessors */
    inline float squared_norm() const {
#ifdef V3F_USE_EIGEN
        return EigenV3f::squaredNorm();
#else
        return data_[0] * data_[0] +
               data_[1] * data_[1] +
               data_[2] * data_[2];
#endif  /* ifdef V3F_USE_EIGEN */
    }

    inline float dist_to(Vector3f const& rhs) const {
#ifdef V3F_USE_EIGEN
        return this->operator-(rhs).norm();
#else
        return sqrt(this->operator-(rhs).squared_norm());
#endif  /* ifdef V3F_USE_EIGEN */
    }
    inline float sq_dist_to(Vector3f const& rhs) const {
        Vector3f const tmp = this->operator-(rhs);
        return tmp.squared_norm();
    }

    /*
     * We use 0.0001 as tolerance here because that's the highest precision
     * PDBs support.
     */
    inline bool is_approx(
        Vector3f const& other,
        float const tolerance = 1e-4) const {
#ifdef V3F_USE_EIGEN
        return isApprox(other, tolerance);
#else
        for (size_t i = 0; i < 3; ++i) {
            if (not float_approximates_err(
                        data_[i],
                        other.data_[i],
                        tolerance)) {
                return false;
            }
        }
        return true;
#endif  /* ifdef V3F_USE_EIGEN */
    }

    bool operator==(Vector3f const& rhs) const { return is_approx(rhs); }

#ifndef V3F_USE_EIGEN

    float operator[](size_t const i) const {
        DEBUG(i >= 3);
        return data_[i];
    }

    float& operator[](size_t const i) {
        DEBUG(i >= 3);
        return data_[i];
    }

    Vector3f operator+(Vector3f const& rhs) const {
        return Vector3f(
                   data_[0] + rhs.data_[0],
                   data_[1] + rhs.data_[1],
                   data_[2] + rhs.data_[2]);
    }

    Vector3f operator-(Vector3f const& rhs) const {
        return Vector3f(
                   data_[0] - rhs.data_[0],
                   data_[1] - rhs.data_[1],
                   data_[2] - rhs.data_[2]);
    }

    template <typename ScalarType>
    Vector3f operator*(ScalarType const scalar) const {
        return Vector3f(
                   data_[0] * scalar,
                   data_[1] * scalar,
                   data_[2] * scalar);
    }

#endif  /* ifndef V3F_USE_EIGEN */

    /* modifiers */
#ifdef V3F_USE_EIGEN
    // Assign Eigen expressions to Vector3f
    template<typename OtherDerived>
    Vector3f& operator=(
        Eigen::MatrixBase<OtherDerived> const& other)
    {
        EigenV3f::operator=(other);
        return *this;
    }
#endif  /* ifdef V3F_USE_EIGEN */

    /* printers */
#ifndef V3F_USE_EIGEN
    std::string to_string() const {
        return string_format("(%f, %f, %f)",
                             data_[0], data_[1], data_[2]);
    }
#endif  /* ifdef V3F_USE_EIGEN */

    std::string to_csv_string() const {
#ifdef V3F_USE_EIGEN
        Vector3f const& data_ = *this;
#endif  /* ifdef V3F_USE_EIGEN */
        return string_format("%f, %f, %f",
                             data_[0], data_[1], data_[2]);
    }

    /* tests */
    static TestStat test();
};

typedef std::vector<Vector3f> V3fList;
typedef Vector3f Mat3f[3] ;

template<typename T>
static inline Vector3f operator*(
    T const& scalar,
    Vector3f const& vec) {
    return vec * scalar;
}

}  /* elfin */

#endif  /* end of include guard: VECTOR3F_H_ */