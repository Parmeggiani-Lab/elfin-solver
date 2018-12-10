#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include <memory>

#include "json.h"
#include "jutil.h"
#include "string_utils.h"
#include "debug_utils.h"

namespace elfin
{

struct Vector3f {
	float x, y, z;

	/* ctors & dtors */
	Vector3f() : Vector3f(0, 0, 0) {}
	Vector3f(const Vector3f & rhs) :
		x(rhs.x), y(rhs.y), z(rhs.z) {}
	Vector3f(float _x, float _y, float _z) :
		x(_x), y(_y), z(_z) {}
	template <typename Iterable>
	Vector3f(Iterable itb) :
		Vector3f(itb.begin(), itb.begin() + 3) {}
	template <typename ItrBegin, typename ItrEnd>
	Vector3f(ItrBegin begin, ItrEnd end) {
		NICE_PANIC((end - begin) < 3,
		           string_format("Invalid Argument Size: %lu, should be <3\n", end - begin));

		auto itr = begin;
		x = *itr++;
		y = *itr++;
		z = *itr++;
	}

	/* other methods */
	std::string to_string() const;
	std::string to_csv_string() const;

	Vector3f operator+(const Vector3f & rhs) const;
	Vector3f operator-(const Vector3f & rhs) const;
	Vector3f operator*(const float f) const;
	Vector3f & operator+=(const Vector3f & rhs);
	Vector3f & operator-=(const Vector3f & rhs);
	float operator[](const size_t idx) {
		DEBUG(idx > 2,
		      string_format("Vector3f operator[] out of bound (max index is 2, but got %lu)", idx));
		return *(&x + idx);
	}
	float operator[](const size_t idx) const {
		DEBUG(idx > 2,
		      string_format("Vector3f operator[] out of bound (max index is 2, but got %lu)", idx));
		return *(&x + idx);
	}
	float dot(const Vector3f & rhs) const;
	float dist_to(const Vector3f & rhs) const;
	float sq_dist_to(const Vector3f & rhs) const;

	// We use 1e-4 because PDBs have only 4 decimals of precision
	bool approximates(const Vector3f & ref, const double tolerance = 1e-4);
};
typedef std::vector<Vector3f> V3fList;

class Transform
{
private:
	/* types */
	struct Data {
		float data[4][4] = {
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 0}
		};
		float * operator[](const size_t i) { return data[i]; }
		const float * operator[](const size_t i) const { return data[i]; }
	};

	/* data */
	Data data_;

	/* modifiers */
	void parse_from_json(const JSON & tx_json);
public:
	/* ctors */
	Transform() {}
	Transform(const Transform & other);
	Transform(Transform && other);
	Transform(const JSON & j);
	Transform(const Data & data);

	/* accessors */
	Transform operator*(const Transform & rhs) const;
	Vector3f operator*(const Vector3f & vec) const;
	Transform inversed() const;
	Vector3f collapsed() const;

	/* modifiers */
	Transform & operator=(const Transform & other);
	Transform & operator=(Transform && other);
	Transform & operator*=(const Transform & rhs);

	/* printers */

	std::string to_string() const;
	std::string to_csv_string() const;
};

} // namespace elfin

#endif /* include guard */