#ifndef GENE_H
#define GENE_H

#include <vector>

#include "shorthands.h"
#include "geometry.h"

namespace elfin
{

class Gene
{
public:
	Gene(const uint _nodeId);

	Gene(const uint _nodeId,
	     const Point3f _com);

	Gene(const uint _nodeId,
	     const float x,
	     const float y,
	     const float z);

	std::string to_string() const;
	std::string to_csv_string() const;
	uint & nodeId();
	const uint & nodeId() const;
	Point3f & com();
	const Point3f & com() const;

	static void setup(const IdNameMap * _inm);

	static const IdNameMap * inm;
private:
	static bool setupDone;

	uint myNodeId;
	Point3f myCom;
};

typedef std::vector<Gene> Genes;
typedef std::vector<Gene>::const_iterator ConstGeneIterator;

std::string genes_to_string(const Genes & genes);
std::string genes_to_csv_string(const Genes & genes);

} // namespace elfin

#endif /* include guard */