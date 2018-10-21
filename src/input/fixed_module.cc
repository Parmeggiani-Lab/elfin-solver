#include "fixed_module.h"

namespace elfin {

std::shared_ptr<FixedModule> FixedModule::from_json(const JSON & j, const std::string & name) {
    std::shared_ptr<FixedModule> fm = std::make_shared<FixedModule>(name);

    return fm;
}
    

}  /* elfin */