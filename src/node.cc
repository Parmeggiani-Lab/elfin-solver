#include "node.h"

#include <sstream>
#include <algorithm>

namespace elfin {

/* public */
Node::Node(const ProtoModule * prototype, const Transform & tx) :
    prototype_(prototype),
    tx_(tx) {
}

std::string Node::to_string() const {
    std::stringstream ss;
    ss << "Node[" << prototype_->name << "]" << std::endl;
    ss << "Tx: " << tx_.to_string();
    return ss.str();
}

std::string Node::to_csv_string() const {
    die("Revise this function?\n");
    return tx_.to_csv_string();
}

}  /* elfin */