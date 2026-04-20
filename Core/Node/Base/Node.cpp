#include "Node.h"
#include <cmath>
#include <stdexcept>

#include "../../Graph/Graph.h"
#include "../../../Helper/SaphirManager.h"

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


MarketConsumer::MarketConsumer()
    : Consumer(),
      market_parent(nullptr),
      instrument(),
      exchange() {}

MarketConsumer::MarketConsumer(const string& instrument,
                               const string& exchange)
    : Consumer(),
      market_parent(nullptr),
      instrument(instrument),
      exchange(exchange) {}

bool MarketConsumer::update() {
    const bool was_valid = this->valid;

    if (!this->market_parent->is_valid()) {
        this->valid = false;
        return was_valid;
    }

    return this->recompute();
}

Market* MarketConsumer::connect(Graph& graph) {
    if (this->instrument.empty()) {
        throw std::runtime_error("MarketConsumer::connect cannot connect without instrument");
    }

    Market* selected_market = nullptr;
    for (const auto& producer_entry : graph.get_producer_container()) {
        auto* market = dynamic_cast<Market*>(producer_entry.second);
        if (!market) {
            continue;
        }
        if (market->get_instrument() != this->instrument) {
            continue;
        }
        if (!this->exchange.empty() && market->get_exchange() != this->exchange) {
            continue;
        }
        selected_market = market;
        break;
    }

    if (!selected_market) {
        if (this->exchange.empty()) {
            throw std::runtime_error(
                "MarketConsumer::connect requires a non-empty exchange to create a Market producer"
            );
        }

        const SaphirManager saphir;
        const double tick_value = saphir.get_market_tick_value(this->exchange, this->instrument);
        selected_market = new Market(
            this->instrument,
            this->exchange,
            static_cast<int>(kBookLevels),
            tick_value
        );
        graph.add_producer(selected_market);
    }

    this->market_parent = selected_market;
    this->instrument = selected_market->get_instrument();
    this->exchange = selected_market->get_exchange();

    graph.add_edge(selected_market, this);
    this->mark_dirty();

    return selected_market;
}

const string& MarketConsumer::get_instrument() const {
    return this->instrument;
}

const string& MarketConsumer::get_exchange() const {
    return this->exchange;
}

Market* MarketConsumer::get_market_parent() const {
    return this->market_parent;
}
