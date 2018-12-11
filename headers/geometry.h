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
	/* data */
	float x, y, z;

	/* ctors */
	Vector3f() : Vector3f(0, 0, 0) {}
	Vector3f(Vector3f const& rhs) :
		x(rhs.x), y(rhs.y), z(rhs.z) {}
	Vector3f(float _x, float _y, float _z) :
		x(_x), y(_y), z(_z) {}
	template <typename Iterable>
	Vector3f(Iterable itb) :
		Vector3f(itb.begin(), itb.begin() + 3) {}
	template <typename ItrBegin, typename ItrEnd>
	Vector3f(ItrBegin begin, ItrEnd end) {
		NICE_PANIC((end - begin) < 3,
		           string_format(
		               "Invalid Argument Size: %lu, should be <3\n",
		               end - begin));

		auto itr = begin;
		x = *itr++;
		y = *itr++;
		z = *itr++;
	}

	/* accessors */
	Vector3f operator+(Vector3f const& rhs) const;
	Vector3f operator-(Vector3f const& rhs) const;
	Vector3f operator*(float const f) const;
	float operator[](size_t const idx) const {
		DEBUG(idx > 2,
		      string_format(
		          ("Vector3f operator[] out of bound "
		           "(max index is 2, but got %lu)"),
		          idx));
		return *(&x + idx);
	}
	float dot(Vector3f const& rhs) const;
	float dist_to(Vector3f const& rhs) const;
	float sq_dist_to(Vector3f const& rhs) const;

	// Default tolerance is 1e-4 because PDBs have only 4 decimals of
	// precision
	bool approximates(
	    Vector3f const& ref,
	    double const tolerance = 1e-4) const;

	/* modifiers */
	Vector3f & operator+=(Vector3f const& rhs);
	Vector3f & operator-=(Vector3f const& rhs);

	/* printers */
	std::string to_string() const;
	std::string to_csv_string() const;

	/* tests */
	static size_t test();
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
		float * operator[](size_t const i) { return data[i]; }
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
	Transform & operator=(Transform const& other);
	Transform & operator=(Transform && other);
	Transform & operator*=(Transform const& rhs);

	/* printers */
	std::string to_string() const;
	std::string to_csv_string() const;

	/* tests */
	static size_t test();
};

} // namespace elfin

#endif /* include guard */