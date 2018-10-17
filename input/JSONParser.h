#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "../src/input/json.h"

#include "SpecParser.h"
#include "DBParser.h"
#include "../data/PairRelationship.h"

namespace elfin
{

// Credits to nolhmann from https://github.com/nlohmann/json
using JSON = nlohmann::json;

class JSONParser : public SpecParser, DBParser
{
public:
	JSONParser() {};
	virtual ~JSONParser() {};

	void parseDB(
	    const std::string & filename,
	    NameIdMap & nameMapOut,
	    IdNameMap & inmOut,
	    RelaMat & relMatOut,
	    RadiiList & radiiListOut);

	Points3f parseSpec(const std::string & filename);

	JSON parse(const std::string & filename);
	void inspect(const std::string & filename);
private:
};

} // namespace Elfin

#endif /* include guard */