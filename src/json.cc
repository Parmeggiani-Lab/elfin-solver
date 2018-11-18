#include "json.h"

#include "jutil.h"

#include <sstream>
#include <fstream>

namespace elfin {

std::string json_to_str(const JSON & j) {
    std::ostringstream ss;
    if (j.is_string()) {
        ss << j.get<std::string>();
    }
    else {
        ss << j;
    }
    return ss.str();
}

JSON parse_json(const std::string & filename)
{
    std::ifstream input_stream(filename);

    panic_if(!input_stream.is_open(),
             "Could not open file: \"%s\"\n", filename.c_str());

    JSON j;
    input_stream >> j;

    return j;
}

}