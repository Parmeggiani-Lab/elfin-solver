#include "ui_object.h"

namespace elfin {

/*
Takes a blender object level JSON object and its blender ID, then parses the
JSON data into elfin representation.
*/
UIObject::UIObject(
    const JSON& j, const std::string& name) :
    tx_(Transform(j["tx"])),
    name_(name) {}

UIObject::UIObject(
    const Transform& tx, const std::string& name) :
    tx_(tx), name_(name) {}

}  /* elfin */