#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include <memory>

#include "json.h"
#include "elfin_exception.h"

namespace elfin
{

typedef std::vector<float>::const_iterator FloatConstIterator;
class Mat3x3;

class Vector3f
{
public:
	float x, y, z;

	Vector3f() : Vector3f(0, 0, 0) {}

	Vector3f(const Vector3f & rhs) :
		x(rhs.x), y(rhs.y), z(rhs.z) {}

	Vector3f(float _x, float _y, float _z) :
		x(_x), y(_y), z(_z) {}

	template <typename Itb>
	Vector3f(Itb itb) :
		Vector3f(itb.begin(), itb.begin() + 3) {}

	template <typename ItrBegin, typename ItrEnd>
	Vector3f(ItrBegin begin, ItrEnd end)
	{
		if ((end - begin) != 3)
			throw InvalidArgumentSize;

		auto itr = begin;
		x = *itr++;
		y = *itr++;
		z = *itr++;
	}

	std::string to_string() const;
	std::string to_csv_string() const;

	Vector3f operator+(const Vector3f & rhs) const;
	Vector3f operator-(const Vector3f & rhs) const;
	Vector3f operator*(const float f) const;
	Vector3f & operator+=(const Vector3f & rhs);
	Vector3f & operator-=(const Vector3f & rhs);
	float dot(const Vector3f & rhs) const;
	Vector3f dot(const Mat3x3 & rotMat) const;
	float dist_to(const Vector3f & rhs) const;
	float sq_dist_to(const Vector3f & rhs) const;

	// We use 1e-6 because PDBs have only 4 decimals of precision
	bool approximates(const Vector3f & ref, double tolerance = 1e-4);
};
typedef Vector3f Point3f;
typedef std::vector<Point3f> Points3f;

std::string points_to_string(const Points3f & points);

class Mat3x3
{
public:
	Vector3f rows[3];

	Mat3x3() : rows( { {0., 0., 0.}, {0., 0., 0.}, {0., 0., 0.}}) {};
	Mat3x3(Vector3f _rows[3]);
	Mat3x3(const std::vector<float> & v) :
		Mat3x3(v.begin(), v.begin() + 9) {}

	template <typename ItrBegin, typename ItrEnd>
	Mat3x3(ItrBegin begin, ItrEnd end)
	{
		if ((end - begin) != 9)
			throw "InvalidArgumentSize";

		ItrBegin itr = begin;
		for (int i = 0; i < 3; i++)
		{
			rows[i] = Vector3f(itr, itr + 3);
			itr += 3;
		}
	}

	template <typename Itrable3x3>
	Mat3x3(Itrable3x3 itb)
	{
		if ((itb.end() - itb.begin()) != 3)
			throw InvalidArgumentSize;
		auto itr = itb.begin();
		for (int i = 0; i < 3; i++)
		{
			rows[i] = Vector3f((*itr).begin(), (*itr).end());
			itr++;
		}
	}

	Vector3f dot(const Vector3f & rotMat) const;
	Mat3x3 dot(const Mat3x3 & rotMat) const;
	Mat3x3 transpose() const;

	std::string to_string() const;
};


} // namespace elfin

#endif /* include guard */