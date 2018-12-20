#ifndef RECIPE_H_
#define RECIPE_H_

#include <vector>
#include <initializer_list>
#include <memory>

#include "terminus_type.h"
#include "geometry.h"

// A class to aid testing Node Teams' correct node sequence construction.
namespace elfin {

class Recipe {
public:
    /* type */
    struct Step {
        std::string name = "not initialized";
        TerminusType out_term = TerminusType::NONE;
        size_t chain_id = 9999;
    };

private:
    class PImpl;
    std::unique_ptr<PImpl> p_impl_;
    
public:
    /* ctors */
    Recipe(std::initializer_list<Step> steps);

    /* dtors */
    virtual ~Recipe();

    /* accessors */
    V3fList const& points() const;

    /* modifiers */

    /* printers */
    
};  /* class Recipe */

}  /* elfin */

#endif  /* end of include guard: RECIPE_H_ */