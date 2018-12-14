#ifndef MATRIX_BASE_ADDONS_H_
#define MATRIX_BASE_ADDONS_H_

/* printers */
IOFormat const CleanFormt =
    IOFormat(4, 0, ", ", "\n", "[", "]");

std::string to_string() const {
    std::ostringstream ss;
    ss << this->format(CleanFormt);
    return ss.str();
}

#endif  /* end of include guard: MATRIX_BASE_ADDONS_H_ */