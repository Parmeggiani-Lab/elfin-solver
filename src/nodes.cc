#include "nodes.h"

namespace elfin {

/* private */
void Nodes::destroy_nodes() {
    for (auto node : *this) {
        delete node;
    }

    this->clear();
}

/* public */
Nodes::Nodes(const Nodes & other) {
    *this = other;
}

Nodes & Nodes::operator=(const Nodes & other) {
    this->destroy_nodes();

    // Make new node pointers
    for (auto node : other) {
        this->push_back(node->clone());
    }
}

Nodes::~Nodes() {
    destroy_nodes();
}

}  /* elfin */