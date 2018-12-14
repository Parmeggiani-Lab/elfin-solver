#ifndef VECTOR3F_H_
#define VECTOR3F_H_

#include <vector>

#include "eigen.h"
#include "debug_utils.h"

namespace elfin {

typedef Eigen::Vector3f EigenV3f;
class Vector3f : public EigenV3f {
private:
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
        Vector3f& me = *this;
        me[0] = *itr++;
        me[1] = *itr++;
        me[2] = *itr++;
    }

public:
    /* ctors */

    // Construct Vector3f from Eigen expressions
    template<typename OtherDerived>
    Vector3f(
        Eigen::MatrixBase<OtherDerived> const& other)
        : EigenV3f(other) {}
    Vector3f() : EigenV3f(EigenV3f::Zero()) {}
    Vector3f(float x, float y, float z) : EigenV3f(x, y, z) {}
    template <typename T>
    Vector3f(std::vector<T> const& vec) :
        Vector3f(vec.cbegin(), vec.cbegin() + 3) {}

    /* accessors */
    inline float dist_to(Vector3f const& rhs) const {
        return this->operator-(rhs).norm();
    }
    inline float sq_dist_to(Vector3f const& rhs) const {
        return this->operator-(rhs).squaredNorm();
    }

    // Default tolerance is 1e-4 because PDBs have only 4 decimals of
    // precision
    inline bool approximates(
        Vector3f const& other,
        double const tolerance = 1e-4) const {
        return isApprox(other, tolerance);
    }

    /* modifiers */
    // Assign Eigen expressions to Vector3f
    template<typename OtherDerived>
    Vector3f& operator=(
        Eigen::MatrixBase<OtherDerived> const& other)
    {
        this->EigenV3f::operator=(other);
        return *this;
    }

    /* printers */
    std::string to_csv_string() const {
        Vector3f const& me = *this;
        return string_format("%f, %f, %f",
                             me[0], me[1], me[2]);
    }

    /* tests */
    static void test(size_t& errors, size_t& tests);
};
typedef std::vector<Vector3f> V3fList;

}  /* elfin */

#endif  /* end of include guard: VECTOR3F_H_ */