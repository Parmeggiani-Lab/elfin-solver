#include <sstream>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "geometry.h"
#include "jutil.h"

namespace elfin {

/* Transform public */
/* Transform ctors */
// Transform::Transform(Transform const& other) {
// 	*this = other; // call operator=(const T&)
// }

// Transform::Transform(Transform && other) {
// 	*this = other; // call operator=(T&&)
// }

// Transform::Transform(JSON const& tx_json) {
// 	size_t i = 0;
// 	for (auto row_json : tx_json) {
// 		size_t j = 0;
// 		for (auto f_json : row_json) {
// 			data_[i][j] = f_json.get<float>();
// 			++j;
// 			DEBUG(j > 4);
// 		}
// 		++i;
// 		DEBUG(i > 4);
// 	}
// }

// /* Transform accessors */
// Transform Transform::operator*(Transform const& rhs) const {
// 	/*
// 	 * In A* B = C, A is this current Transform.
// 	 */
// 	Transform new_tx;
// 	for (size_t i = 0; i < 4; ++i) {
// 		for (size_t j = 0; j < 4; ++j) {
// 			new_tx.data_[i][j] = data_[i][0] * rhs.data_[0][j] +
// 			                     data_[i][1] * rhs.data_[1][j] +
// 			                     data_[i][2] * rhs.data_[2][j] +
// 			                     data_[i][3] * rhs.data_[3][j];
// 		}
// 	}

// 	return new_tx;
// }

// Vector3f Transform::operator*(Vector3f const& vec) const {
// 	float const x = vec[0] + data_[3][0];
// 	float const y = vec[1] + data_[3][1];
// 	float const z = vec[2] + data_[3][2];

// 	float const rx = data_[0][0] * x +
// 	                 data_[0][1] * y +
// 	                 data_[0][2] * z +
// 	                 data_[0][3];

// 	float const ry = data_[1][0] * x +
// 	                 data_[1][1] * y +
// 	                 data_[1][2] * z +
// 	                 data_[1][3];

// 	float const rz = data_[2][0] * x +
// 	                 data_[2][1] * y +
// 	                 data_[2][2] * z +
// 	                 data_[2][3];

// 	return Vector3f(rx, ry, rz);
// }

// Transform Transform::inversed() const {
// 	Transform new_tx;
// 	for (size_t i = 0; i < 4; ++i) {
// 		for (size_t j = i; j < 4; ++j) {
// 			new_tx.data_[i][j] = data_[j][i];
// 			new_tx.data_[j][i] = data_[i][j];
// 		}
// 	}
// 	return new_tx;
// }

// Vector3f Transform::collapsed() const {
// 	return this->operator*(Vector3f());
// }

// /* Transform modifiers */
// Transform& Transform::operator=(Transform const& other) {
// 	data_ = other.data_;
// 	return *this;
// }

// Transform& Transform::operator=(Transform && other) {
// 	data_ = other.data_;
// 	return *this;
// }

// Transform& Transform::operator*=(Transform const& rhs) {
// 	/*
// 	 * A *= B => A = B* A
// 	 * A is this current Transform.
// 	 */
// 	for (size_t i = 0; i < 4; ++i) {
// 		for (size_t j = 0; j < 4; ++j) {
// 			data_[i][j] = rhs.data_[i][0] * data_[0][j] +
// 			              rhs.data_[i][1] * data_[1][j] +
// 			              rhs.data_[i][2] * data_[2][j] +
// 			              rhs.data_[i][3] * data_[3][j];
// 		}
// 	}

// 	return *this;
// }

// bool Transform::operator==(Transform const& other) const {
// 	return data_ == other.data_;
// }

// /* Transform printers */
// std::string Transform::to_string() const {
// 	std::ostringstream ss;

// 	ss << "tx[\n";
// 	for (size_t i = 0; i < 4; ++i) {
// 		ss << "\t[ ";
// 		for (size_t j = 0; j < 4; ++j) {
// 			ss << data_[i][j];
// 			if (j < 3) {
// 				ss << ", ";
// 			}
// 		}
// 		ss << " ]\n";
// 	}
// 	ss << "  ]";

// 	return ss.str();
// }

// std::string Transform::to_csv_string() const
// {
// 	return collapsed().to_csv_string();
// }

// /* Transform tests */
// // static
// size_t Transform::test() {
// 	UNIMPLEMENTED();

// 	// Test ctors, accessors and modifiers
// }

} // namespace elfin