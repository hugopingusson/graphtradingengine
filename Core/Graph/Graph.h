//
// Created by hugo on 25/03/25.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <cstdint>
#include <map>
#include <vector>
#include "../Node/Base/Node.h"

using namespace std;

class Graph {
    public:
    Graph();
    Graph(Logger* logger);

    int64_t get_last_graph_latency() const;
    Logger* get_logger();
    map<int,vector<int>> get_adjacency_map();
    map<int,Node*> get_child_node_container();
    map<int,SourceNode*> get_source_container();
    map<int,Node*> get_output_container();
    map<int,vector<int>> get_update_path();

    bool empty() const;
    int get_node_id(Node* node) const;
    ///////////////////////////////////// GRAPH CONSTRUCTION LOGIC //////////////////////////


    bool checked_in(Node* node);

    void add_source(SourceNode* source_node);
    void add_edge(Node* publisher,Node* subscriber);
    void resolve_output_nodes();
    void resolve_update_path();
    vector<vector<int>> link(int target_node_id);

    ///////////////////////////////////// GRAPH RUNNING LOGIC //////////////////////////

    void update(const int& source_id);


    protected:

    int64_t sequence_number;
    int64_t last_in_streamer_timestamp;
    int64_t last_in_order_gateway_timestamp;
    int64_t last_in_graph_timestamp;
    int64_t last_out_graph_timestamp;
    int max_id;

    Logger* logger;

    map<int,vector<int>> adjacency_map;
    map<int,Node*> child_node_container;
    map<int,SourceNode*> source_container;
    map<int,Node*> output_container;
    map<int,vector<int>> update_path;
};



#endif //GRAPH_H
