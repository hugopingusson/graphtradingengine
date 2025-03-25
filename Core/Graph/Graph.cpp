//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

Graph::Graph():sequence_number(int64_t()),last_exchange_timestamp(int64_t()),last_reception_timestamp(int64_t()),last_in_graph_timestamp(int64_t()),last_out_graph_timestamp(int64_t()),adjacency_map(map<int,vector<int>>()),node_container(map<int,vector<Node*>>()),source_container(map<int,vector<Node*>>()) {}

int64_t Graph::get_last_graph_latency() {
    return last_out_graph_timestamp - last_in_graph_timestamp;
}

