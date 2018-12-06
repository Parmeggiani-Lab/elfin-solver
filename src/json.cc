#include "json.h"

#include <sstream>
#include <fstream>

#include "jutil.h"
#include "debug_utils.h"

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

    NICE_PANIC(!input_stream.is_open(),
               string_format("Could not open file: \"%s\"\n", filename.c_str()));

    JSON j;
    input_stream >> j;

    return j;
}

}