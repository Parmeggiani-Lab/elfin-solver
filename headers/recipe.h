#ifndef RECIPE_H_
#define RECIPE_H_

namespace elfin {

namespace tests {

struct RecipeStep {
    std::string const mod_name;
    TerminusType const src_term = TerminusType::NONE;
    std::string const src_chain;
    std::string const dst_chain;
    std::string const ui_name;
};
typedef std::vector<RecipeStep> Recipe;

}  /* tests */

}  /* elfin */

#endif  /* end of include guard: RECIPE_H_ */