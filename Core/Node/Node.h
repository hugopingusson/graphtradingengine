//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_NODE_H
#define CLASS_NODE_H
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../Logger/Logger.h"

using namespace std;

class Node {
    public:
    virtual ~Node() = default;


    Node();
    explicit Node(const string& name);
    explicit Node(const int& node_id,const string& name,Logger* main_logger);

    virtual void update()=0;

    [[nodiscard]] string get_name() const;
    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] int64_t get_last_reception_timestamp() const;
    [[nodiscard]] int64_t get_last_exchange_timestamp() const;

    void set_name(const string& name);

    void set_logger(Logger* main_logger);
    void set_node_id(const int& node_id);

    protected:
    int node_id;
    int64_t sequence_number;
    string name;
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;
    bool valid;
    Logger* main_logger;


};


class ValueNode: public Node {
    public:
    ~ValueNode() override = default;

    ValueNode();
    explicit ValueNode(const int& node_id,const string& name,Logger* logger);

    void update() override =0;

    // ValidCollector get_valid_collector() const;
    [[nodiscard]] double get_last_value() const;



    protected:
        double last_value;


};


class MonoValueNode: public ValueNode {
    public:
    ~MonoValueNode() override = default;
    MonoValueNode();
    MonoValueNode(Node* parent);
    explicit MonoValueNode(const int& node_id,const string& name,Node* parent,Logger* logger);
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

// class MultiValueNode: public ValueNode {
//     public:
//     ~MultiValueNode() override = default;
//     MultiValueNode();
//     MultiValueNode(const string& name);
//
//     virtual double compute(const Node& node)=0;
//     void update(const Node& node);
//
//     protected:
//     ValidCollector valid_collector;
// };

#endif //CLASS_NODE_H
