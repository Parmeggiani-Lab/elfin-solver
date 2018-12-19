#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>

#include "json.h"
#include "geometry.h"
#include "map_utils.h"

namespace elfin {

class UIObject
{
protected:
    std::string name_ = "unamed_ui_object";
    Transform tx_;

public:
    UIObject(JSON const& j, const std::string& name);
    UIObject(Transform const& tx, const std::string& name);
    virtual ~UIObject() {}

    // getters
    const std::string& name() const { return name_; }
    Transform const& tx() const { return tx_; }
};

typedef SPMap<UIObject> UIObjectMap;

}

#endif  /* end of include guard: UI_OBJECT_H_ */