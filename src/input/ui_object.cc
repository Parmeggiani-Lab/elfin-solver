#include "ui_object.h"

namespace elfin {

/*
Takes a blender object level JSON object and its blender ID, then parses the
JSON data into elfin representation.
*/
UIObject::UIObject(
    const JSON & j, const std::string & name) :
    rot_(Mat3x3(j["rot"])),
    tran_(Point3f(j["trans"])),
    name_(name) {}

UIObject::UIObject(
    const Mat3x3 & rot, const Vector3f & tran, const std::string & name) :
    rot_(rot), tran_(tran), name_(name) {}

}  /* elfin */