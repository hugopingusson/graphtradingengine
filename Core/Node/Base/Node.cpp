#include "Node.h"
#include <cmath>

Node::Node() {
    sequence_number = int64_t();
    last_order_gateway_in_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    valid=false;
    name=string();
    logger=nullptr;
    node_id=int();
}

Node::Node(const string& name) : Node() {
    this->name=name;
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

int64_t Node::get_last_reception_timestamp() const {
    return this->last_reception_timestamp;
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

Producer::Producer() = default;

Consumer::Consumer() : dirty(false) {}

bool Consumer::is_dirty() const {
    return this->dirty;
}

void Consumer::mark_dirty() {
    this->dirty = true;
}

void Consumer::clear_dirty() {
    this->dirty = false;
}

ProducerConsumer::ProducerConsumer() = default;


///////////////////////////////////////////////////////////////////////////////////////////////////////
Quote::Quote(): ask_price(),bid_price(){}
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

SingleInputConsumer::SingleInputConsumer(Node *parent) : parent(parent), value(std::nan("")) {}


double SingleInputConsumer::get_value() const {
    return this->value;
}

Node* SingleInputConsumer::get_parent() const {
    return this->parent;
}

bool SingleInputConsumer::update() {
    if (!this->parent) {
        const bool was_valid = this->valid;
        this->valid = false;
        return was_valid;
    }

    if (!this->parent->is_valid()) {
        const bool was_valid = this->valid;
        this->valid = false;
        return was_valid;
    }

    const bool was_valid = this->valid;
    const double last_value = this->value;
    this->compute();

    if (this->valid != was_valid) {
        return true;
    }
    if (!this->valid) {
        return false;
    }
    return this->value != last_value;
}
