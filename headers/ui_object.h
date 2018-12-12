#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>
#include <unordered_map>

#include "json.h"
#include "geometry.h"

namespace elfin {

class UIObject
{
protected:
    std::string name_ = "unamed_ui_object";
    Transform tx_;

public:
    UIObject(JSON const& j, const std::string & name);
    UIObject(Transform const& tx, const std::string & name);
    virtual ~UIObject() {}

    // getters
    const std::string & name() const { return name_; }
    Transform const& tx() const { return tx_; }
};

typedef std::unordered_map<std::string, UIObject> UIObjects;

}

#endif  /* end of include guard: UI_OBJECT_H_ */