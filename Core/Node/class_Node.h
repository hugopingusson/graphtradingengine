//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_NODE_H
#define CLASS_NODE_H
#include <cstdint>
#include <string>
using namespace std;

class Node {
    public:
    virtual ~Node() = 0;

    Node();
    Node(const string& name);


    virtual void update(const Node& node)=0;

    protected:
    int64_t sequence_number;
    string name;
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;
    bool valid;


};

#endif //CLASS_NODE_H
