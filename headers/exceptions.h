#ifndef EXIT_EXCEPTION_H_
#define EXIT_EXCEPTION_H_

#include <exception>

namespace elfin {

using namespace std;

struct ElfinException : public runtime_error {
    using runtime_error::runtime_error;
};

struct ExitException : public ElfinException {
    int const code;
    ExitException(int const _code, string const& reason) :
        ElfinException(reason),
        code(_code) {}
};

#define DECL_EXCEPTION(NAME) \
struct NAME : public ElfinException {\
    using ElfinException::ElfinException;\
    NAME() : ElfinException("Unknwon") {}\
};


DECL_EXCEPTION(BadArgument);
DECL_EXCEPTION(BadXDB);
DECL_EXCEPTION(BadTerminus);
DECL_EXCEPTION(BadWorkType);
DECL_EXCEPTION(InvalidHinge);
DECL_EXCEPTION(OutOfRange);
DECL_EXCEPTION(ValueNotFound);
DECL_EXCEPTION(ShouldNotReach);
DECL_EXCEPTION(Unsupported);
DECL_EXCEPTION(CouldNotParse);

#undef DECL_EXCEPTION

}  /* elfin */

#endif  /* end of include guard: EXIT_EXCEPTION_H_ */