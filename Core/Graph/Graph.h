//
// Created by hugo on 25/03/25.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <cstdint>
#include <map>
#include <vector>
#include "../Node/Node.h"

using namespace std;

class Graph {
    public:
    Graph();


    int64_t get_last_graph_latency();
    // int64_t get_last_engine_graph_latency();
    // int64_t get_feed_handler_graph_latency();

    void add_source(Node* source_node);
    void make_subscription(Node*);


    protected:
    int64_t sequence_number;
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;
    int64_t last_in_graph_timestamp;
    int64_t last_out_graph_timestamp;


    map<int,vector<int>> adjacency_map;
    map<int,vector<Node*>> node_container;
    map<int,vector<Node*>> source_container;







};



#endif //GRAPH_H
