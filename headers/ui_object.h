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
    std::string name_;
    Mat3x3 rot_;
    Vector3f tran_;

public:
    UIObject(const JSON & j, const std::string & name);
    UIObject(const Mat3x3 & rot, const Vector3f & tran,
             const std::string & name);
    virtual ~UIObject() {}

    // getters
    const std::string & name() const { return name_; }
    const Mat3x3 & rot() const { return rot_; }
    const Vector3f & tran() const { return tran_; }
};

typedef std::vector<UIObject> UIObjects;

}

#endif  /* end of include guard: UI_OBJECT_H_ */