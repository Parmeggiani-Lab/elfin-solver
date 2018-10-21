#ifndef ELFIN_EXCEPTION_H_
#define ELFIN_EXCEPTION_H_

namespace elfin {

// Exception wrapper
class ElfinException : public std::exception
{
    const std::string msg_;
public:
    ElfinException(const std::string & msg) : msg_(msg) {}
    ~ElfinException() throw () {}
    virtual const char* what() const throw() { return msg_.c_str(); }
};

const static ElfinException InvalidArgumentSize("Invalid Argument Size");

}  /* elfin */

#endif  /* end of include guard: ELFIN_EXCEPTION_H_ */