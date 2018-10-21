#ifndef JSON_H_
#define JSON_H_

#include "nlohmann_json.h"

#include <string>

namespace elfin {

typedef nlohmann::json JSON;

std::string json_to_str(const JSON & j);

JSON parse_json(const std::string & filename);

}

#endif /* end of include guard: JSON_H_ */