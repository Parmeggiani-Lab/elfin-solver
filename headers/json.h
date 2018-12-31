#ifndef JSON_H_
#define JSON_H_

#include <string>

#include "nlohmann/json.hpp"

namespace elfin {

typedef nlohmann::json JSON;

class RelaMat;

std::string json_to_clean_str(JSON const& j);

JSON parse_json(std::string const& filename);

}

#endif /* end of include guard: JSON_H_ */