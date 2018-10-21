#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include <memory>

#include "json.h"

namespace elfin
{

typedef std::vector<float>::const_iterator FloatConstIterator;
class Mat3x3;

class Vector3f
{
public:
	float x, y, z;

	Vector3f(const Vector3f & rhs);

	Vector3f();

	Vector3f(float _x, float _y, float _z);

	Vector3f(const std::vector<float> & v);

	Vector3f(FloatConstIterator begin,
	         FloatConstIterator end);

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

	static std::shared_ptr<Vector3f> from_json(const JSON & j);
};
typedef Vector3f Point3f;
typedef std::vector<Point3f> Points3f;

std::string points_to_string(const Points3f & points);

class Mat3x3
{
public:
	Vector3f rows[3];

	Mat3x3(Vector3f _rows[3]);

	Mat3x3(const std::vector<float> & v) :
		Mat3x3(v.begin(), v.end())
	{}

	Mat3x3(FloatConstIterator begin,
	       FloatConstIterator end);

	Vector3f dot(const Vector3f & rotMat) const;
	Mat3x3 dot(const Mat3x3 & rotMat) const;
	Mat3x3 transpose() const;

	std::string to_string() const;
};


} // namespace elfin

#endif /* include guard */