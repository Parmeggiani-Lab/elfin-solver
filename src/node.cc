#include "node.h"

#include <sstream>

namespace elfin {

std::string Node::to_string() const {
    std::stringstream ss;
    ss << "Name: " << prototype->name_ << std::endl;
    ss << "Tx: " << tx.to_string();
    return ss.str();
}

std::string Node::to_csv_string() const {
    return tx.to_csv_string();
}

}  /* elfin */