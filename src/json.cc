#include "json.h"

#include <sstream>
#include <fstream>

#include "jutil.h"
#include "debug_utils.h"

namespace elfin {

std::string json_to_clean_str(JSON const& j) {
    std::ostringstream ss;
    if (j.is_string()) {
        ss << j.get<std::string>();
    }
    else {
        ss << j;
    }
    return ss.str();
}

JSON parse_json(std::string const& filename)
{
    std::ifstream input_stream(filename);

    TRACE(not input_stream.is_open(),
          "Could not open JSON file: \"%s\"\n",
          filename.c_str());

    JSON j;
    input_stream >> j;

    return j;
}

}