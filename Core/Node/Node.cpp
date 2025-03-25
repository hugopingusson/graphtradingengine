#include "Node.h"
#include <cmath>

Node::Node() {
    sequence_number = int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    valid=false;
    name=string();
    main_logger=new Logger();
    node_id=int();
}

Node::Node(const int& node_id,const string &name,Logger* logger) {
    this->node_id=node_id
    sequence_number = int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    valid=false;
    this->name=name;
    main_logger=logger;
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


///////////////////////////////////////////////////////////////////////////////////////////////////////

ValueNode::ValueNode():Node(),last_value(std::nan("")){}
ValueNode::ValueNode(const int& node_id,const string &name,Logger* logger):Node(node_id,name,logger),last_value(std::nan("")) {}


double ValueNode::get_last_value() const {
    return this->last_value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


MonoValueNode::MonoValueNode(const int& node_id,const string &name,Node* parent,Logger* logger):ValueNode(node_id,name,logger),parent(parent) {}

void MonoValueNode::update() {
    if ((*this->parent).is_valid()) {
        this->last_value=this->compute();
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

///////////////////////////////////////////////////////////////////////////////////////////////////////

// ValidCollector::ValidCollector():valid_map(map<const Node*, bool>()) {}
// ValidCollector::ValidCollector(const map<const Node *, bool>& valid_map):valid_map(valid_map) {}
//
//
// void ValidCollector::update(const Node& node) {
//     this->valid_map.at(&node)=node.is_valid();
// }
//
//
//
// MultiValueNode::MultiValueNode():ValueNode(){
//     valid_collector = ValidCollector();
//
// }
//
// MultiValueNode::MultiValueNode(const string &name):ValueNode(name) {
//     valid_collector = ValidCollector();
// }
//
// void MultiValueNode::update(const Node& node) {
//     this->valid_collector.update(node);
//
// }
