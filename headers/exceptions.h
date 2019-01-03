#ifndef EXIT_EXCEPTION_H_
#define EXIT_EXCEPTION_H_

#include <exception>

namespace elfin {

//
using namespace std;

struct ExitException : public runtime_error {
    int const code;
    ExitException(int const _code, string const& reason) :
        runtime_error(reason),
        code(_code) {}
};

struct InvalidHingeException : public runtime_error {
    using runtime_error::runtime_error;
};

struct ValueNotFoundException : public runtime_error {
    using runtime_error::runtime_error;
};

}  /* elfin */

#endif  /* end of include guard: EXIT_EXCEPTION_H_ */