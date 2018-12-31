#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>

#include "json.h"
#include "geometry.h"
#include "map_utils.h"

namespace elfin {

struct UIObject {
    /* data */
    std::string const name;
    Transform const tx;

    UIObject(std::string const& _name,
             Transform const& _tx) :
        name(_name), tx(_tx) {}
    UIObject(std::string const& _name,
             JSON const& json) :
        UIObject(_name, Transform(json)) {}
};

typedef SPMap<UIObject> UIObjectMap;

}

#endif  /* end of include guard: UI_OBJECT_H_ */