#include "string_utils.h"

#include <sstream>

namespace elfin {

std::string Printable::to_string() const {
	std::ostringstream oss;
	print_to(oss);
	return oss.str();
}

} /* elfin */

namespace std {

std::ostream& operator<<(std::ostream& os, elfin::Printable const& b) {
    b.print_to(os);
	return os;
}

} /* std */