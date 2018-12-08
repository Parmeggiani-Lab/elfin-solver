#include <sstream>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "geometry.h"
#include "jutil.h"

namespace elfin
{

typedef uint32_t uint;

std::string Vector3f::to_string() const
{
	std::ostringstream ss;
	ss << "v3f[" << std::setprecision(10) << x << ", " << y << ", " << z << ']';
	return ss.str();
}

std::string Vector3f::to_csv_string() const
{
	std::ostringstream ss;
	ss << std::setprecision(10) << x << ", " << y << ", " << z;
	return ss.str();
}

Vector3f Vector3f::operator+(const Vector3f & rhs) const
{
	return Vector3f(
	           rhs.x + this->x,
	           rhs.y + this->y,
	           rhs.z + this->z);
}

Vector3f Vector3f::operator-(const Vector3f & rhs) const
{
	return Vector3f(
	           this->x - rhs.x,
	           this->y - rhs.y,
	           this->z - rhs.z);
}

Vector3f Vector3f::operator*(const float f) const
{
	return Vector3f(
	           f * this->x,
	           f * this->y,
	           f * this->z);
}

Vector3f & Vector3f::operator+=(const Vector3f& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;

	return *this;
}

Vector3f & Vector3f::operator-=(const Vector3f& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;

	return *this;
}

float Vector3f::dot(const Vector3f & rhs) const
{
	return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
}

float Vector3f::dist_to(const Vector3f & rhs) const
{
	return sqrt(sq_dist_to(rhs));
}

float Vector3f::sq_dist_to(const Vector3f & rhs) const
{
	const float dx = (this->x - rhs.x);
	const float dy = (this->y - rhs.y);
	const float dz = (this->z - rhs.z);
	return dx * dx + dy * dy + dz * dz;
}

bool Vector3f::approximates(const Vector3f & ref, const double tolerance)
{
	if (this->x != ref.x ||
	        this->y != ref.y ||
	        this->z != ref.z)
	{
		const float dx = this->x - ref.x;
		const float dy = this->y - ref.y;
		const float dz = this->z - ref.z;

		wrn("Vector3f ref: %s\ntest: %s\ndiffs: %.8f, %.8f, %.8f\n",
		    ref.to_string().c_str(),
		    to_string().c_str(),
		    dx, dy, dz);

		if (!float_approximates_err(
		            dx, 0.0, tolerance) ||
		        !float_approximates_err(
		            dy, 0.0, tolerance) ||
		        !float_approximates_err(
		            dz, 0.0, tolerance))
		{
			return false;
		}
	}

	return true;
}

/* Transform */

/* private */
void Transform::parse_from_json(const JSON & tx_json) {
	size_t i = 0;
	for (auto row_json : tx_json) {
		size_t j = 0;
		for (auto f_json : row_json) {
			data_[i][j] = f_json.get<float>();
			++j;
			DEBUG(j > 4);
		}
		++i;
		DEBUG(i > 4);
	}
}

/* public */
/* ctors */
Transform::Transform(const Transform & other) {
	*this = other; // call operator=(const T&)
}

Transform::Transform(Transform && other) {
	*this = other; // call operator=(T&&)
}

Transform::Transform(const JSON & j) {
	parse_from_json(j);
}

Transform::Transform(const Data & data) : data_(data) {
}

/* accessors */
Transform Transform::operator*(const Transform & rhs) const {
	/*
	 * In A * B = C, A is this current Transform.
	 */
	Data data;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			data[i][j] = data_[i][0] * rhs.data_[0][j] +
			            data_[i][1] * rhs.data_[1][j] +
			            data_[i][2] * rhs.data_[2][j] +
			            data_[i][3] * rhs.data_[3][j];
		}
	}

	return Transform(data);
}

Vector3f Transform::operator*(const Vector3f & vec) const {
	const float x = vec[0] + data_[3][0];
	const float y = vec[1] + data_[3][1];
	const float z = vec[2] + data_[3][2];

	const float rx = data_[0][0] * x +
	                 data_[0][1] * y +
	                 data_[0][2] * z +
	                 data_[0][3];

	const float ry = data_[1][0] * x +
	                 data_[1][1] * y +
	                 data_[1][2] * z +
	                 data_[1][3];

	const float rz = data_[2][0] * x +
	                 data_[2][1] * y +
	                 data_[2][2] * z +
	                 data_[2][3];

	return Vector3f(rx, ry, rz);
}

Transform Transform::inversed() const {
	Data data;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = i + 1; j < 4; ++j) {
			data[i][j] = data_[j][i];
		}
	}
	return Transform(data);
}

Vector3f Transform::collapsed() const {
	return this->operator*(Vector3f());
}

/* modifiers */
Transform & Transform::operator=(const Transform & other) {
	data_ = other.data_;
	return *this;
}

Transform & Transform::operator=(Transform && other) {
	data_ = other.data_;
	return *this;
}

Transform & Transform::operator*=(const Transform & rhs) {
	/*
	 * A *= B => A = B * A
	 * A is this current Transform.
	 */
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			data_[i][j] = rhs.data_[i][0] * data_[0][j] +
			                  rhs.data_[i][1] * data_[1][j] +
			                  rhs.data_[i][2] * data_[2][j] +
			                  rhs.data_[i][3] * data_[3][j];
		}
	}

	return *this;
}

/* printers */
std::string Transform::to_string() const {
	std::ostringstream ss;

	ss << "tx[\n";
	for (size_t i = 0; i < 4; ++i) {
		ss << "\t[ ";
		for (size_t j = 0; j < 4; ++j) {
			ss << data_[i][j];
			if (j < 3) {
				ss << ", ";
			}
		}
		ss << " ]\n";
	}
	ss << "  ]";

	return ss.str();
}

std::string Transform::to_csv_string() const
{
	return collapsed().to_csv_string();
}

} // namespace elfin