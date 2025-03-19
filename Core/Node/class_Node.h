//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_NODE_H
#define CLASS_NODE_H
#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace std;

class Node {
    public:
    virtual ~Node() = default;


    Node();
    Node(const string& name);

    virtual void update(const Node& node)=0;

    string get_name() const;
    bool is_valid() const;
    int64_t get_last_reception_timestamp() const;
    int64_t get_last_exchange_timestamp() const;


    protected:
    int64_t sequence_number;
    string name;
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;
    bool valid;


};

class ValidCollector {
public:

    ValidCollector();
    ValidCollector(const map<const Node*, bool>& valid_map);

    void update(const Node& node);


protected:
    map<const Node*, bool> valid_map;

};


class ValueNode: public Node {
    public:
    virtual ~ValueNode() = default;

    ValueNode();
    ValueNode(const string& name);

    virtual void update(const Node& node)=0;

    ValidCollector get_valid_collector() const;
    double get_last_value() const;



    protected:
        double last_value;


};


class MonoValueNode: public ValueNode {
    public:
    virtual ~MonoValueNode() = default;
    MonoValueNode();
    MonoValueNode(const string& name);
    virtual double compute(const Node& node)=0;
    void update(const Node& node);

};


class MultiValueNode: public ValueNode {
    public:
    virtual ~MultiValueNode() = default;
    MultiValueNode();
    MultiValueNode(const string& name);

    virtual double compute(const Node& node)=0;
    void update(const Node& node);

    protected:
    ValidCollector valid_collector;
};

#endif //CLASS_NODE_H
