#include "spec.h"

namespace elfin {

Spec::Spec(const JSON & j) {
    
    // place holder
    for (int i = 0; i < 5; i++)
        this->emplace_back(10*i, 15*i, 20*i);
}

} /* elfin */