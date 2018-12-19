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

    UIObject(
        Transform const& _tx,
        const std::string& _name) :
        tx(_tx), name(_name) {}
    UIObject(
        JSON const& json,
        const std::string& _name) :
        UIObject(Transform(json), _name) {}
};

typedef SPMap<UIObject> UIObjectMap;

}

#endif  /* end of include guard: UI_OBJECT_H_ */