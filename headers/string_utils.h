#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <unordered_map>

namespace elfin {

typedef std::vector<std::string> StrList;
typedef std::unordered_map<std::string, size_t> StrIndexMap;

/*
 * From
 * https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 * By iFreilicht
 */
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

}  /* elfin */

#endif  /* end of include guard: STRING_UTILS_H_ */