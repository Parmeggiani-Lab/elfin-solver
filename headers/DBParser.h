#ifndef DBPARSER_H_
#define DBPARSER_H_

#include <string>

#include "shorthands.h"
#include "radii.h"
#include "PairRelationship.h"

namespace elfin
{

class DBParser
{
public:
	DBParser() {};
	virtual ~DBParser() {};

	// Might add a parseStream in the future if ever needed
	virtual void parseDB(
	    const std::string & filename,
	    NameIdMap & nameMapOut,
	    IdNameMap & inmOut,
	    RelaMat & relMatOut,
	    RadiiList & radiiListOut) = 0;

private:
};

} // namespace elfin

#endif /* include guard */