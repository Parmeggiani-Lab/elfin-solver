#ifndef FIXED_MODULE_H_
#define FIXED_MODULE_H_

#include <memory>
#include <string>

#include "ui_object.h"

namespace elfin {

class FixedModule : public UIObject<FixedModule>
{
public:
    using UIObject::UIObject;
    
    static std::shared_ptr<FixedModule> from_json(const JSON & j, const std::string & name);
};

}  /* elfin */

#endif  /* end of include guard: FIXED_MODULE_H_ */