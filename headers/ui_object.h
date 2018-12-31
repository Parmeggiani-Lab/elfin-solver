#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>

#include "json.h"
#include "geometry.h"
#include "map_utils.h"
#include "string_utils.h"

namespace elfin {

struct UIObject : public Printable {
    /* data */
    std::string const name;
    Transform const tx;

    /* ctors */
    UIObject(std::string const& _name, Transform const& _tx);
    UIObject(std::string const& _name, JSON const& json);

    /* dtors */
    virtual ~UIObject() {};
    
    /* printers */
    virtual void print_to(std::ostream& os) const;
};

typedef SPMap<UIObject> UIObjectMap;

}

#endif  /* end of include guard: UI_OBJECT_H_ */