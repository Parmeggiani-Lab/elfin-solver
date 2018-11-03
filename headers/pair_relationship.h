#ifndef PAIRRELATIONSHIP_H
#define PAIRRELATIONSHIP_H

#include <vector>
#include <sstream>

#include "geometry.h"

namespace elfin
{

class PairRelationship
{
public:
	PairRelationship(
	    const std::vector<float> & com_b,
	    const std::vector<float> & rot,
	    const std::vector<float> & tran) :
		com_b_(com_b),
		rot_(rot),
		rot_inv_(rot_.transpose()),
		tran_(tran)
	{};

	virtual ~PairRelationship() {};

	const Point3f com_b_;
	const Mat3x3 rot_;
	const Mat3x3 rot_inv_;
	const Vector3f tran_;

	std::string to_string()
	{
		std::ostringstream ss;

		ss << "pr[" << std::endl;
		ss << "    com_b_:" << com_b_.to_string() << std::endl;
		ss << "    rot_:" << rot_.to_string() << std::endl;
		ss << "    rot_inv_:" << rot_inv_.to_string() << std::endl;
		ss << "    tran_:" << tran_.to_string() << std::endl;
		ss << "]" << std::endl;

		return ss.str();
	}
};

typedef std::vector<PairRelationship *> RelaRow;
class RelationshipMatrix : public std::vector<RelaRow>
{
public:
	~RelationshipMatrix() {
		for(auto & row : *this) {
			while(row.size()) {
				auto pr = row.back();
				row.pop_back();
				delete pr;
			}
		}
	};
};

extern const RelationshipMatrix & REL_MAT;

} // namespace elfin

#endif /* include guard */