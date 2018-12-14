#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vector3f.h"
#include "transform.h"

namespace elfin {

// class Transform {
// private:
// 	/* types */
// 	struct Data {
// 		float data[4][4] = {
// 			{1, 0, 0, 0},
// 			{0, 1, 0, 0},
// 			{0, 0, 1, 0},
// 			{0, 0, 0, 0}
// 		};
// 		float* operator[](size_t const i) { return data[i]; }
// 		float const* operator[](size_t const i) const { return data[i]; }
// 		bool operator==(Data const& other) const {
// 			return memcmp(data, other.data, sizeof(data)) == 0;
// 		}
// 	};

// 	/* data */
// 	Data data_;

// public:
// 	/* ctors */
// 	Transform() {}
// 	Transform(Transform const& other);
// 	Transform(Transform && other);
// 	Transform(JSON const& tx_json);

// 	/* accessors */
// 	Transform operator*(Transform const& rhs) const;
// 	Vector3f operator*(Vector3f const& vec) const;
// 	Transform inversed() const;
// 	Vector3f collapsed() const;
// 	bool operator==(Transform const& other) const;
// 	bool operator!=(Transform const& other) const { return not this->operator==(other); }

// 	/* modifiers */
// 	Transform& operator=(Transform const& other);
// 	Transform& operator=(Transform && other);
// 	Transform& operator*=(Transform const& rhs);

// 	/* printers */
// 	std::string to_string() const;
// 	std::string to_csv_string() const;

// 	/* tests */
// 	static size_t test();
// };

} // namespace elfin

#endif /* include guard */