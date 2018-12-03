#include "nodes.h"

namespace elfin {

/* public */
Nodes::Nodes(const Nodes & other) {
    *this = other;
}

Nodes & Nodes::operator=(const Nodes & other) {
    clear();

    // Make new node pointers
    for (auto node : other) {
        this->push_back(node->clone());
    }

    return *this;
}

Nodes::~Nodes() {
    clear();
}

void Nodes::clear() {
    for (auto node : *this) {
        delete node;
    }

    NodesContainer::clear();
}

}  /* elfin */