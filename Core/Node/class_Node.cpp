//
// Created by hugo on 16/03/25.
//

#include "class_Node.h"

Node::Node() {
    sequence_number = int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    valid=false;
    name=string();
}

Node::Node(const string &name) {
    sequence_number = int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    valid=false;
    this->name=name;
}
