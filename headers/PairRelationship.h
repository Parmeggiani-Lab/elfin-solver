#ifndef PAIRRELATIONSHIP_H
#define PAIRRELATIONSHIP_H

#include <vector>
#include <sstream>

#include "elfin_types.h"

namespace elfin
{

class PairRelationship
{
public:
	PairRelationship(
	    const std::vector<float> & comBv,
	    const std::vector<float> & rotv,
	    const std::vector<float> & tranv) :
		comB(Point3f(comBv)),
		rot(Mat3x3(rotv)),
		rotInv(rot.transpose()),
		tran(Vector3f(tranv))
	{};

	virtual ~PairRelationship() {};

	const Point3f comB;
	const Mat3x3 rot;
	const Mat3x3 rotInv;
	const Vector3f tran;

	std::string toString()
	{
		std::ostringstream ss;

		ss << "pr[" << std::endl;
		ss << "    comB:" << comB.toString() << std::endl;
		ss << "    rot:" << rot.toString() << std::endl;
		ss << "    rotInv:" << rotInv.toString() << std::endl;
		ss << "    tran:" << tran.toString() << std::endl;
		ss << "]" << std::endl;

		return ss.str();
	}
};

typedef std::vector<PairRelationship *> RelaRow;
class RelaMat : public std::vector<RelaRow>
{
public:
	~RelaMat() {
		for(auto & row : *this) {
			while(row.size()) {
				auto pr = row.back();
				row.pop_back();
				delete pr;
			}
		}
	};
};

} // namespace elfin

#endif /* include guard */