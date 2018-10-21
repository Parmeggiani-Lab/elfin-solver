#ifndef RADII_H_
#define RADII_H_

#include <vector>

namespace elfin {

struct Radii {
    float avgAll;
    float maxCA;
    float maxHeavy;
    Radii(float aa, float mca, float mh) :
        avgAll(aa), maxCA(mca), maxHeavy(mh)
    {}
};
typedef std::vector<Radii> RadiiList;

}  /* elfin */

#endif  /* end of include guard: RADII_H_ */