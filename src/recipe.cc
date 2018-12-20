#include "recipe.h"

#include "debug_utils.h"

namespace elfin {
/* private */
struct Recipe::PImpl {
    /* data */
    std::vector<Step> steps;
    V3fList points;

};

/* public */
/* ctors */
Recipe::Recipe(std::initializer_list<Step> steps) {
    p_impl_ = std::make_unique<PImpl>();
    p_impl_->steps = steps;
}

/* dtors */
Recipe::~Recipe() {}

/* accessors */
V3fList const& Recipe::points() const {
    return p_impl_->points;
}

}  /* elfin */