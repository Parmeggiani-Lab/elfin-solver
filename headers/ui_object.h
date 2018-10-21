#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>
#include <memory>
#include <vector>

#include "json.h"
#include "geometry.h"

namespace elfin {

class UIObject
{
public:
    const std::string name_;
    const Mat3x3 rot_;
    const Vector3f tran_;

    UIObject(const JSON & j, const std::string & name);
    virtual ~UIObject() {}

    // getters
    const Mat3x3 & get_rot() { return rot_; }
    const Vector3f & get_tran() { return tran_; }
};

typedef std::vector<UIObject> UIObjects;

}

#endif  /* end of include guard: UI_OBJECT_H_ */