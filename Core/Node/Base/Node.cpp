#include "Node.h"
#include <cmath>

Node::Node() {
    sequence_number = int64_t();
    last_order_gateway_in_timestamp=int64_t();
    last_streamer_in_timestamp=int64_t();
    last_capture_server_in_timestamp=int64_t();
    valid=false;
    name=string();
    logger=nullptr;
    node_id=int();
    time_helper=TimeHelper();
}

Node::Node(const string& name) {
    sequence_number = int64_t();
    last_order_gateway_in_timestamp=int64_t();
    last_streamer_in_timestamp=int64_t();
    last_capture_server_in_timestamp=int64_t();
    valid=false;
    this->name=name;
    logger=nullptr;
    node_id=int();
    time_helper=TimeHelper();
}


string Node::get_name() const {
    return this->name;
}

int Node::get_node_id() const {
    return this->node_id;
}

bool Node::is_valid() const {
    return this->valid;
}

int64_t Node::get_last_order_gateway_in_timestamp() const {
    return this->last_order_gateway_in_timestamp;
}

int64_t Node::get_last_streamer_in_timestamp() const {
    return this->last_streamer_in_timestamp;
}

int64_t Node::get_last_capture_server_in_timestamp() const {
    return this->last_capture_server_in_timestamp;
}


void Node::set_name(const string& name) {
    this->name=name;
}

void Node::set_logger(Logger* main_logger) {
    this->logger=main_logger;
}

void Node::set_node_id(const int& node_id) {
    this->node_id=node_id;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
Quote::Quote(): ask_price(),bid_price(){};
// Quote::Quote(const string& name):Node(name),ask_price(),bid_price(){};
// Quote::Quote(const int& node_id,const string& name,Logger* main_logger):ChildNode(node_id,name,main_logger),ask_price(),bid_price(){};


double Quote::get_ask_price() {
    return ask_price;
}

double Quote::get_bid_price() {
    return bid_price;
}


double Quote::mid() {
    return 0.5*(ask_price+bid_price);
}

double Quote::spread() {
    return ask_price-bid_price;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

SingleInputConsumer::SingleInputConsumer(): parent(nullptr),value(std::nan("")){}
// Signal::Signal(const int& node_id,const string &name,Logger* logger):ChildNode(node_id,name,logger),value(std::nan("")) {}

SingleInputConsumer::SingleInputConsumer(Node *parent) : value(std::nan("")) {
    this->parent = parent;
}


double SingleInputConsumer::get_value() const {
    return this->value;
}

Node* SingleInputConsumer::get_parent() const {
    return this->parent;
}

bool SingleInputConsumer::forward() {
    if (this->valid) {
        if (!this->parent->is_valid()) {
            this->valid=false;
            return true;
        }
        else {
            double last_value=this->value;
            this->compute();
            if (this->valid) {
                if (last_value!=this->value) {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return true;
            }
        }
    }
    else {
        if (!this->parent->is_valid()) {
            return false;
        }
        else {
            this->compute();
            if (this->valid) {
                return true;
            }
            else {
                return false;
            }
        }
    }
}


// if ((*this->parent).is_valid()) {
//     if (this->valid) {
//         double last_value=this->value;
//         this->compute();
//         if (this->valid) {
//             if (this->value != last_value) {
//                 return true;
//             }
//             else{
//                 return false;
//             }
//         }
//         else {
//             return true;
//         }
//     }
//     else {
//         this->compute();
//         if (this->valid) {
//         }
//     }
// }
//





///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Signal::Signal(): value(std::nan("")){}
// // Signal::Signal(const int& node_id,const string &name,Logger* logger):ChildNode(node_id,name,logger),value(std::nan("")) {}
//
//
// double Signal::get_value() const {
//     return this->value;
// }
//
// ///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MonoSignal::MonoSignal() {
//     parent=nullptr;
// };
//
// MonoSignal::MonoSignal(Node* parent) {
//     this->parent=parent;
// }
//
// // MonoSignal::MonoSignal(const int& node_id,const string &name,Node* parent,Logger* logger):Signal(node_id,name,logger),parent(parent) {}
//
// Node *MonoSignal::get_parent() const {
//     return this->parent;
// }
//
//
// void MonoSignal::update() {
//     if ((*this->parent).is_valid()) {
//         this->value=this->compute();
//         if (std::isnan(this->value)) {
//             this->valid=false;
//         }
//         else {
//             this->valid=true;
//         }
//     }else {
//         this->valid=false;
//     }
// }

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
// MultiValueNode::MultiValueNode():Signal(){
//     valid_collector = ValidCollector();
//
// }
//
// MultiValueNode::MultiValueNode(const string &name):Signal(name) {
//     valid_collector = ValidCollector();
// }
//
// void MultiValueNode::update(const Node& node) {
//     this->valid_collector.update(node);
//
// }
