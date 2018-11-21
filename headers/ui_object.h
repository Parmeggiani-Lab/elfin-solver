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
    std::string name_;
    Transform tx_;

public:
    UIObject(const JSON & j, const std::string & name);
    UIObject(const Transform & tx, const std::string & name);
    virtual ~UIObject() {}

    // getters
    const std::string & name() const { return name_; }
    const Transform & tx() const { return tx_; }
};

typedef std::unordered_map<std::string, UIObject> UIObjects;

}

#endif  /* end of include guard: UI_OBJECT_H_ */