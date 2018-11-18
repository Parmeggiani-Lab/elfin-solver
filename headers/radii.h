#ifndef RADII_H_
#define RADII_H_

#include <vector>

namespace elfin {

struct Radii {
    float avg_all;
    float max_ca;
    float max_heavy;
    
    Radii(float aa, float mca, float mh) :
        avg_all(aa), max_ca(mca), max_heavy(mh)
    {}
};
typedef std::vector<Radii> RadiiList;

extern const RadiiList & RADII_LIST; // defined in elfin.cc

}  /* elfin */

#endif  /* end of include guard: RADII_H_ */