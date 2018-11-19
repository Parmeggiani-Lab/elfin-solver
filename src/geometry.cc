#include <sstream>
#include <cmath>
#include <iomanip>

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

Vector3f Vector3f::dot(const Mat3x3 & mat) const
{
	const float rx = this->x * mat.rows[0].x +
	                 this->y * mat.rows[1].x +
	                 this->z * mat.rows[2].x;

	const float ry = this->x * mat.rows[0].y +
	                 this->y * mat.rows[1].y +
	                 this->z * mat.rows[2].y;

	const float rz = this->x * mat.rows[0].z +
	                 this->y * mat.rows[1].z +
	                 this->z * mat.rows[2].z;

	return Vector3f(rx, ry, rz);
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

bool Vector3f::approximates(const Vector3f & ref, double tolerance)
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

/* ctors & dtors */
Transform::Transform(const JSON & j) {
	ElementArray ele = { 0 };
	parse_elements_from_json(j, ele);
	init_with_elements(ele);
}

void Transform::parse_elements_from_json(const JSON & j, ElementArrayRef ele) const {
	size_t i = 0, j = 0;
	for (auto row_it : j) {
		for (auto f_it : row_it.second) {
			ele[i][j] = (*f_it.second).get<float>();
			++j;
		}
		++i;
	}
}

Transform::Transform(const ElementArrayRef ele) {
	init_with_elements(ele);
}

void Transform::init_with_elements(const ElementArrayRef ele) {
	std::copy(&ele[0][0], &ele[0][0] + (16 * sizeof(float)), &elements_[0][0]);
}

/* other methods */
Transform operator*(const Transform & tx_b) const {
	/*
	 * In A * B = C, A is this current Transform.
	 */
	ElementArray ele = { 0 };
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			ele[i][j] = elements_[i][0] * tx_b.elements_[0][j] +
			            elements_[i][1] * tx_b.elements_[1][j] +
			            elements_[i][2] * tx_b.elements_[2][j] +
			            elements_[i][3] * tx_b.elements_[3][j];
		}
	}

	return Transform(ele);
}

Transform & operator*=(const Transform & tx_b) {
	/*
	 * A *= B => A = B * A
	 * A is this current Transform.
	 */
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			elements_[i][j] = tx_b.elements_[i][0] * elements_[0][j] +
			                  tx_b.elements_[i][1] * elements_[1][j] +
			                  tx_b.elements_[i][2] * elements_[2][j] +
			                  tx_b.elements_[i][3] * elements_[3][j];
		}
	}

	return *this;
}

Transform operator*(const Vector3f & vec) const {
	const float x = vec[0] + elements_[3][0];
	const float y = vec[1] + elements_[3][1];
	const float z = vec[2] + elements_[3][2];

	const float rx = elements_[0][0] * x +
	                 elements_[0][1] * y +
	                 elements_[0][2] * z +
	                 elements_[0][3];

	const float ry = elements_[1][0] * x +
	                 elements_[1][1] * y +
	                 elements_[1][2] * z +
	                 elements_[1][3];

	const float rz = elements_[2][0] * x +
	                 elements_[2][1] * y +
	                 elements_[2][2] * z +
	                 elements_[2][3];

	return Vector3f(rx, ry, rz);
}

Transform inversed() const {
	ElementArray ele;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = i + 1; j < 4; ++j) {
			ele[i][j] = elements_[j][i];
		}
	}
	return Transform(ele);
}

std::string to_string() const {
	std::ostringstream ss;

	ss << "tx[\n"
	for (size_t i = 0; i < 4; ++i) {
		ss << "\t[ ";
		for (size_t j = 0; j < 4; ++j) {
			ss << elements_[i][j];
			if (j < 3) {
				ss << ", ";
			}
		}
		ss << " ]\n";
	}
	ss << "  ]";

	return ss.str();
}

} // namespace elfin