#include "ui_object.h"

namespace elfin {

/*
Takes a blender object level JSON object and its blender ID, then parses the
JSON data into elfin representation.
*/
UIObject::UIObject(
    JSON const& j,
    std::string const& name) :
    tx_(Transform(j)),
    name_(name) {}

UIObject::UIObject(
    Transform const& tx,
    std::string const& name) :
    tx_(tx),
    name_(name) {}

}  /* elfin */