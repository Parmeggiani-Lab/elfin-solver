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

#define DECL_EXCEPTION(NAME) \
struct NAME : public runtime_error {\
    using runtime_error::runtime_error;\
    NAME() : runtime_error("Unknwon") {}\
};


DECL_EXCEPTION(BadArgument);
DECL_EXCEPTION(BadTerminus);
DECL_EXCEPTION(BadWorkType);
DECL_EXCEPTION(InvalidHinge);
DECL_EXCEPTION(ValueNotFound);
DECL_EXCEPTION(ShouldNotReach);
DECL_EXCEPTION(CouldNotParse);

#undef DECL_EXCEPTION

}  /* elfin */

#endif  /* end of include guard: EXIT_EXCEPTION_H_ */