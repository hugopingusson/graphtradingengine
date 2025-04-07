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

using namespace std;

class Node {
    public:
    virtual ~Node() = default;


    Node();
    explicit Node(const string& name);
    explicit Node(const int& node_id,const string& name,Logger* main_logger);


    [[nodiscard]] string get_name() const;
    [[nodiscard]] int get_node_id() const;
    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] int64_t get_last_reception_timestamp() const;
    [[nodiscard]] int64_t get_last_exchange_timestamp() const;
    [[nodiscard]] int64_t get_last_adapter_timestamp() const;

    void set_name(const string& name);

    void set_logger(Logger* main_logger);
    void set_node_id(const int& node_id);

    protected:
    int node_id;
    int64_t sequence_number;
    string name;
    int64_t last_streamer_in_timestamp; // reception timestamp
    int64_t last_order_gateway_in_timestamp; //exchange timestamp
    int64_t last_capture_server_in_timestamp;
    bool valid;
    Logger* logger;


};

class SourceNode:public Node {
    public:
    virtual ~SourceNode() = default;
    SourceNode();
    explicit SourceNode(const string& name);


};


class ChildNode:public Node {
    public:
    virtual ~ChildNode() = default;
    ChildNode();
    ChildNode(const int& node_id,const string& name,Logger* main_logger);
    explicit ChildNode(const string& name);

    virtual void update()=0;

};

class Quote : public ChildNode {
    public:
    ~Quote() override = default;

    Quote();
    explicit Quote(const string& name);
    explicit Quote(const int& node_id,const string& name,Logger* main_logger);

    double get_ask_price();
    double get_bid_price();

    double mid();
    double spread();

    void update() override =0;

    protected:
    double ask_price;
    double bid_price;



};

class Signal: public ChildNode {
    public:
    ~Signal() override = default;
    Signal();
    explicit Signal(const int& node_id,const string& name,Logger* logger);



    virtual double compute()=0;
    void update() override =0;

    [[nodiscard]] double get_value() const;



    protected:
    double value;


};

class MonoSignal: public Signal {
    public:
    ~MonoSignal() override = default;
    MonoSignal();
    MonoSignal(Node* parent);
    explicit MonoSignal(const int& node_id,const string& name,Node* parent,Logger* logger);
    virtual double compute()=0;


    void update() override;
    Node* get_parent() const;

    protected:
    Node* parent;

};

// class ValidCollector {
// public:
//
//     ValidCollector();
//     explicit ValidCollector(const map<const Node*, bool>& valid_map);
//
//     void update(const Node& node);
//
//
// protected:
//     map<const Node*, bool> valid_map;
//
// };

// class MultiValueNode: public Signal {
//     public:
//     ~MultiValueNode() override = default;
//     MultiValueNode();
//     MultiValueNode(const string& name);
//
//     virtual double compute(const Node& node)=0;
//     void update(const Node& node);
//
//     protected:ll
//     ValidCollector valid_collector;
// };

#endif //CLASS_NODE_H
