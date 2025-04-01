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


    int64_t get_last_graph_latency() const;
    // int64_t get_last_engine_graph_latency();
    // int64_t get_feed_handler_graph_latency();
    Logger get_logger();

    bool empty() const;
    int get_node_id(Node* node) const;


    void checkin(Node* node);
    bool checked_in(Node* node);

    void add_source(Node* source_node);
    void add_edge(Node* publisher,Node* subscriber);
    void resolve_output_nodes();
    // void add_mono_value_node(Node* subscriber);

    void resolve_update_path();

    vector<vector<int>> link(int target_node_id);

    protected:

    int64_t sequence_number;
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;
    int64_t last_in_graph_timestamp;
    int64_t last_out_graph_timestamp;
    int max_id;

    Logger logger;

    map<int,vector<int>> adjacency_map;
    map<int,Node*> node_container;
    map<int,Node*> source_container;
    map<int,Node*> output_container;
    map<int,vector<int>> update_path;
};



#endif //GRAPH_H
