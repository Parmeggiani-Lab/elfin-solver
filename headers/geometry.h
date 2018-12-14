#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include <memory>

#include <Eigen/Dense>

#include "json.h"
#include "debug_utils.h"

namespace elfin
{

typedef Eigen::Vector3f EigenV3f;
class Vector3f : public EigenV3f {
private:
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

	// This constructor allows you to construct Vector3f from Eigen expressions
	template<typename OtherDerived>
	Vector3f(const Eigen::MatrixBase<OtherDerived>& other)
		: EigenV3f(other)
	{ }
	// This method allows you to assign Eigen expressions to Vector3f
	template<typename OtherDerived>
	Vector3f& operator=(const Eigen::MatrixBase <OtherDerived>& other)
	{
		this->EigenV3f::operator=(other);
		return *this;
	}

	Vector3f() : EigenV3f(0.f, 0.f, 0.f) {}
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
	bool approximates(
	    Vector3f const& other,
	    double const tolerance = 1e-4) const {
		for (size_t i = 0; i < 3; ++i) {
			if (not float_approximates_err(
			            this->operator[](i), other[i], tolerance)) {
				return false;
			}
		}
		return true;
	}

	/* printers */
	std::string to_string() const {
		Vector3f const& me = *this;
		return string_format("(%f, %f, %f)",
		                     me[0], me[1], me[2]);
	}
	std::string to_csv_string() const {
		Vector3f const& me = *this;
		return string_format("%f, %f, %f",
		                     me[0], me[1], me[2]);
	}

	/* tests */
	static size_t test();
};
typedef std::vector<Vector3f> V3fList;

class Transform {
private:
	/* types */
	struct Data {
		float data[4][4] = {
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 0}
		};
		float* operator[](size_t const i) { return data[i]; }
		float const* operator[](size_t const i) const { return data[i]; }
		bool operator==(Data const& other) const {
			return memcmp(data, other.data, sizeof(data)) == 0;
		}
	};

	/* data */
	Data data_;

public:
	/* ctors */
	Transform() {}
	Transform(Transform const& other);
	Transform(Transform && other);
	Transform(JSON const& tx_json);

	/* accessors */
	Transform operator*(Transform const& rhs) const;
	Vector3f operator*(Vector3f const& vec) const;
	Transform inversed() const;
	Vector3f collapsed() const;
	bool operator==(Transform const& other) const;
	bool operator!=(Transform const& other) const { return not this->operator==(other); }

	/* modifiers */
	Transform& operator=(Transform const& other);
	Transform& operator=(Transform && other);
	Transform& operator*=(Transform const& rhs);

	/* printers */
	std::string to_string() const;
	std::string to_csv_string() const;

	/* tests */
	static size_t test();
};

} // namespace elfin

#endif /* include guard */