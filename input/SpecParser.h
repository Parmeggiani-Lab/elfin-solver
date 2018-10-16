#ifndef SPECPARSER_H_
#define SPECPARSER_H_

#include <vector>
#include <string>

#include "../src/elfin_types.h"

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