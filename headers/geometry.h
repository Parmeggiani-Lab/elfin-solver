#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include <memory>

#include "json.h"
#include "elfin_exception.h"
#include "jutil.h"

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
		if ((end - begin) < 3)
			throw InvalidArgumentSize;

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
		panic_when(idx > 2);
		return *(&x + idx);
	}
	float operator[](const size_t idx) const {
		panic_when(idx > 2);
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
public:
	/* types */
	typedef float ElementArray[4][4];
	typedef ElementArray & ElementArrayRef;
private:
	/* data members */
	ElementArray elements_ = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 0}
	};
public:
	/* ctors & dtors */
	Transform() {}
	Transform(const JSON & j);
	void parse_elements_from_json(const JSON & tx_json, ElementArrayRef ele) const;
	Transform(const ElementArrayRef ele);
	void init_with_elements(const ElementArrayRef ele);

	/* other methods */
	std::string to_string() const;
	std::string to_csv_string() const;
	Transform operator*(const Transform & tx) const;
	Transform & operator*=(const Transform & tx);
	Vector3f operator*(const Vector3f & vec) const;
	Transform inversed() const;
	Vector3f collapsed() const;
};

} // namespace elfin

#endif /* include guard */