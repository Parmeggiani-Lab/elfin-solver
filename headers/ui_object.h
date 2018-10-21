#ifndef UI_OBJECT_H_
#define UI_OBJECT_H_

#include <string>
#include <memory>

#include "json.h"

namespace elfin {

template<typename DT>
class UIObject
{
public:
    const std::string name_;

    UIObject() = delete;
    UIObject(const std::string & name) : name_(name) {}
    virtual ~UIObject() {}

    static std::shared_ptr<DT> from_json(const JSON & j, const std::string & name) {
        return DT::from_json();
    }
};

}

#endif  /* end of include guard: UI_OBJECT_H_ */