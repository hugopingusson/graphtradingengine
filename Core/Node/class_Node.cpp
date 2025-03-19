//
// Created by hugo on 16/03/25.
//

#include "class_Node.h"

#include <math.h>

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

string Node::get_name() const {
    return this->name;
}

bool Node::is_valid() const {
    return this->valid;
}

int64_t Node::get_last_exchange_timestamp() const {
    return this->last_exchange_timestamp;
}

int64_t Node::get_last_reception_timestamp() const {
    return this->last_reception_timestamp;
}



ValidCollector::ValidCollector():valid_map(map<const Node*, bool>()) {}
ValidCollector::ValidCollector(const map<const Node *, bool>& valid_map):valid_map(valid_map) {}


void ValidCollector::update(const Node& node) {
    this->valid_map.at(&node)=node.is_valid();
}


ValueNode::ValueNode():Node(),last_value(std::nan("")){}
ValueNode::ValueNode(const string &name):Node(name),last_value(std::nan("")) {}


double ValueNode::get_last_value() const {
    return this->last_value;
}


MonoValueNode::MonoValueNode():ValueNode() {}
MonoValueNode::MonoValueNode(const string &name):ValueNode(name) {}

void MonoValueNode::update(const Node &node) {
    if (node.is_valid()) {
        this->last_value=this->compute(node);
        if (std::isnan(this->last_value)) {
            this->valid=false;
        }
        else {
            this->valid=true;
        }
    }else {
        this->valid=false;
    }
}



MultiValueNode::MultiValueNode():ValueNode(){
    valid_collector = ValidCollector();

}

MultiValueNode::MultiValueNode(const string &name):ValueNode(name) {
    valid_collector = ValidCollector();
}

void MultiValueNode::update(const Node& node) {
    this->valid_collector.update(node);

}
