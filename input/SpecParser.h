#ifndef SPECPARSER_H_
#define SPECPARSER_H_

#include <vector>
#include <string>

#include "../data/TypeDefs.h"

namespace elfin
{

class SpecParser
{
public:
	SpecParser() {};
	virtual ~SpecParser() {};

	// Might add a parseStream in the future if ever needed
	virtual Points3f parseSpec(
	    const std::string & filename) = 0;

private:
};

} // namespace ElfinParser

#endif /* include guard */