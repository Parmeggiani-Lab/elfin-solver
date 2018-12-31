#include "ui_object.h"

namespace elfin {


/* ctors */
UIObject::UIObject(std::string const& _name,
                   Transform const& _tx) :
    name(_name), tx(_tx) {}

UIObject::UIObject(std::string const& _name,
                   JSON const& json) :
    UIObject(_name, Transform(json)) {}
    
/* printers */
void UIObject::print_to(std::ostream& os) const {
    os << "UIObject (" << name << ") [\n";
    os << tx << "\n";
    os << "]";
}

}  /* elfin */