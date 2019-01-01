#ifndef RECIPE_H_
#define RECIPE_H_

namespace elfin {

namespace tests {

struct RecipeStep {
    std::string mod_name;
    TerminusType src_term = TerminusType::NONE;
    std::string src_chain;
    std::string dst_chain;
};
typedef std::vector<RecipeStep> Recipe;

}  /* tests */

}  /* elfin */

#endif  /* end of include guard: RECIPE_H_ */