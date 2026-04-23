//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_NODE_H
#define CLASS_NODE_H
#include <cstddef>
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

    bool is_dirty() const;
    void mark_dirty();
    void clear_dirty();

    virtual void on_event(Event* event)=0;

protected:
    bool dirty;
};

class Consumer: public virtual Node {
public:
    ~Consumer() override = default;
    Consumer();

    bool is_scheduled() const;
    void mark_scheduled();
    void clear_scheduled();

    bool is_dirty() const;
    void mark_dirty();
    void clear_dirty();

    virtual bool update()=0;

    // update executes the computation and returns true if state changed (value/validity), false otherwise
    // virtual bool forward() = 0;

protected:
    bool scheduled;
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

class Signal : public Consumer {
public:
    ~Signal() override = default;
    Signal();
    Signal(const string& name,
           const size_t& slot_count);

    [[nodiscard]] size_t get_slot_count() const;

    [[nodiscard]] bool is_active(const size_t& index) const;
    void set_active(const size_t& index, const bool& active);

    [[nodiscard]] const vector<double>& get_values() const;
    [[nodiscard]] double get_value(const size_t& index) const;
    [[nodiscard]] const vector<uint8_t>& get_active_mask() const;
    [[nodiscard]] const vector<uint8_t>& get_valid_mask() const;
    [[nodiscard]] bool is_slot_valid(const size_t& index) const;

protected:
    void set_slot_output(const size_t& index,
                         const double& value,
                         const bool& valid);
    bool refresh_global_validity();

    vector<double> values;
    vector<uint8_t> active_mask;
    vector<uint8_t> valid_mask;
};

class ParamStorage {
public:
    ParamStorage();
    ParamStorage(const size_t& hyperparameter_dimension,
                 const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;

    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    size_t hyperparameter_dimension;
    size_t hyperparameter_vector_count;
    vector<double> hyperparameter_space;
};

class ParametrizedSignal : public Signal {
public:
    ~ParametrizedSignal() override = default;
    ParametrizedSignal();
    ParametrizedSignal(const string& name,
                       const size_t& hyperparameter_dimension,
                       const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;
    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    ParamStorage param_storage;
};

class MarketSignal : public Signal {
public:
    ~MarketSignal() override = default;
    MarketSignal();
    MarketSignal(const string& name,
                 const string& instrument,
                 const string& exchange,
                 const size_t& slot_count);

    Market* connect(Graph& graph);
    bool update() override;

    [[nodiscard]] const string& get_instrument() const;
    [[nodiscard]] const string& get_exchange() const;
    [[nodiscard]] Market* get_market_parent() const;

protected:
    virtual bool compute_active_slot(const size_t& index,
                                     double& out_value) = 0;

    Market* market_parent;
    string instrument;
    string exchange;
};

class ParametrizedMarketSignal : public MarketSignal {
public:
    ~ParametrizedMarketSignal() override = default;
    ParametrizedMarketSignal();
    ParametrizedMarketSignal(const string& name,
                             const string& instrument,
                             const string& exchange,
                             const size_t& hyperparameter_dimension,
                             const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;
    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    bool compute_active_slot(const size_t& index,
                             double& out_value) override;
    virtual bool compute_active_vector(const size_t& index,
                                       const double* hyperparameter_vector,
                                       double& out_value) = 0;

    ParamStorage param_storage;
};

#endif //CLASS_NODE_H
