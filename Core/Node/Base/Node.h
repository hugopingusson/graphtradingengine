//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_NODE_H
#define CLASS_NODE_H
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../Logger/Logger.h"
#include "../../Graph/Event.h"
#include "../../../Helper/TimeHelper.h"

using namespace std;

class Graph;
class Market;

class Node {
    public:
    Node();
    explicit Node(const string& name);
    virtual ~Node() = default;
    // explicit Node(const int& node_id,const string& name,Logger* main_logger);


    [[nodiscard]] string get_name() const;
    [[nodiscard]] int get_node_id() const;
    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] int64_t get_last_reception_timestamp() const;
    [[nodiscard]] int64_t get_last_order_gateway_in_timestamp() const;

    void set_name(const string& name);
    void set_logger(Logger* main_logger);
    void set_node_id(const int& node_id);

    protected:
    int node_id;
    int64_t sequence_number;
    string name;
    int64_t last_reception_timestamp;
    int64_t last_order_gateway_in_timestamp; // exchange timestamp
    bool valid;
    Logger* logger;


};

// ---------------------------------------------------------------------------
// Producer / Consumer / ProducerConsumer (replace former SourceNode/ChildNode)
// ---------------------------------------------------------------------------

class Producer: public virtual Node {
public:
    ~Producer() override = default;
    Producer();
    virtual void on_event(Event* event)=0;
};

class Consumer: public virtual Node {
public:
    ~Consumer() override = default;
    Consumer();

    bool is_dirty() const;
    void mark_dirty();
    void clear_dirty();

    virtual bool update()=0;

    // update executes the computation and returns true if state changed (value/validity), false otherwise
    // virtual bool forward() = 0;

protected:
    bool dirty;
};

class ProducerConsumer: public Producer, public Consumer {
public:
    ~ProducerConsumer() override = default;
    ProducerConsumer();
};


class Quote : public Consumer {
    public:
    ~Quote() override = default;
    Quote();
    // explicit Quote(const string& name);
    // explicit Quote(const int& node_id,const string& name,Logger* main_logger);

    double get_ask_price();
    double get_bid_price();

    double mid();
    double spread();

    // bool forward() override = 0;
    bool update() override = 0;

    protected:
    double ask_price;
    double bid_price;

};


// class SingleInputConsumer: public Consumer {
// public:
//     ~SingleInputConsumer() = default;
//     SingleInputConsumer();
//     SingleInputConsumer(Node* parent);
//     // explicit Signal(const int& node_id,const string& name,Logger* logger);
//
//
//
//     virtual void compute()=0;
//     bool update() override;
//
//     [[nodiscard]] double get_value() const;
//
//     Node* get_parent() const;
//
// protected:
//     Node* parent;
//     double value;
// };

class MarketConsumer : public Consumer {
public:
    ~MarketConsumer() override = default;
    MarketConsumer();
    explicit MarketConsumer(const string& instrument,
                            const string& exchange);

    Market* connect(Graph& graph);
    bool update() override;

    [[nodiscard]] const string& get_instrument() const;
    [[nodiscard]] const string& get_exchange() const;
    [[nodiscard]] Market* get_market_parent() const;

protected:
    virtual bool recompute() = 0;

    Market* market_parent;
    string instrument;
    string exchange;
};

#endif //CLASS_NODE_H
