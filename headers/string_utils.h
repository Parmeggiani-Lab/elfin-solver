#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <unordered_map>

namespace elfin {

typedef std::vector<std::string> StrList;
typedef std::unordered_map<std::string, size_t> StrIndexMap;

/*
 * From
 * https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 * By iFreilicht
 */
// template<typename ... Args>
// std::string string_format( const std::string& format, Args ... args )
// {
//     size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
//     std::unique_ptr<char[]> buf(new char[size]);
//     snprintf(buf.get(), size, format.c_str(), args ...);
//     return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
// }

/*
 * Lambda version so compiler does not complain about FMT not being a compile
 * time literal.
 */
#define string_format(FMT, ...) \
[&](){ \
    size_t const size = snprintf(nullptr, 0, FMT, ##__VA_ARGS__) + 1; \
    std::unique_ptr<char[]> buf(new char[size]); \
    snprintf(buf.get(), size, FMT, ##__VA_ARGS__); \
    return std::string(buf.get(), buf.get() + size - 1); \
}()

char const * const unit_tests_passed_str = 
".//////:-.`-\e[1;33mh\e[0m\e[1;33my\e[0mo/----.:////////////////////::o++++++++oooo++`\n.//////-/+-.\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[0m/:.-////////////////////:-oo++oooooo+++++`\n./////::ooo::\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[0m/:////////////////////-/o+o++/+o\e[1;33ms\e[0m\e[1;33my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m.\n.////:-/++++/+\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0mo/://///////////////:-/++\e[1;33ms\e[0m\e[1;33my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m.\n.///:.-::--:/:/\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[0m+///////////////++o\e[1;33my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33ms\e[0m`\n.///-.----------+\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m\e[1;33ms\e[0m+`\n.//:.------------:\e[1;33mo\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[0m/:::`\n.//-.---------..----\e[1;33mo\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m+:::---`\n./:......----..``.-:\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m+::::----`\n.:..---..----.``.-.\e[1;33mo\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[30mo+\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m\e[1;30my\e[1;30m/\e[1;30m+\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m/-::::::`\n.-`--.......----:-:\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33ms\e[30m/:.+\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[1;30m+\e[1;30m/\e[1;30m.\e[1;30m.\e[1;33ms\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[0m-//////`\n.`...```````---::-\e[1;33mo\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[0m\e[1;33ms\e[33mo\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m\e[1;33ms\e[0m\e[1;33ms\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m+/+////`\n`.--.``````.-:::-\e[31m.\e[31m/\e[31m0\e[1;33ms\e[0m\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[30mo\e[1;33ms\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33my\e[0m\e[31my\e[0m\e[31my\e[0m\e[31my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33ms\e[0m/++++/`\n`------:::::::::.\e[31m-\e[31m+\e[31m+\e[31m+\e[31m+\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[0m\e[31m++++o\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m//++//`\n`----------::::-.\e[31m:\e[31m/\e[31m+\e[31m+\e[31mo\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[1;33m+\e[35m/+ooo\e[1;33ms\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[31mo+++++\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[0m:////`\n.:-------------.-\e[1;33my\e[0m\e[1;33ms\e[0m\e[1;33ms\e[0m\e[1;33my\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[35m/oyyyyo\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33ms\e[0m\e[1;33ms\e[0m\e[1;33ms\e[0m\e[1;33ms\e[0m\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m+////`\n-:---------------o\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[35moossoo\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33my\e[0m:+//`\n.--://///++++////:o\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33mh\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33mh\e[0m/:::`\n.------:://////:::/\e[1;33my\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0m\e[1;33md\e[0mo:::`\n`---------:///////:o\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33my\e[0m\e[1;33ms\e[0m---`\n";


class Printable {
public:
   virtual void print_to(std::ostream& os) const = 0;
   std::string to_string() const;
};

}  /* elfin */

namespace std {

std::ostream& operator<<(std::ostream& os, elfin::Printable const& b);

} /* std */

#endif  /* end of include guard: STRING_UTILS_H_ */