#ifndef JSON_H_
#define JSON_H_

#include <string>

#include "nlohmann_json/nlohmann_json.h"

namespace elfin {

typedef nlohmann::json JSON;

class RelaMat;

std::string json_to_str(JSON const& j);

JSON parse_json(const std::string& filename);

}

#endif /* end of include guard: JSON_H_ */