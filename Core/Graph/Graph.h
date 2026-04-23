//
// Created by hugo on 25/03/25.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "../Node/Base/MarketNode.h"
#include "../Node/Base/Node.h"

using namespace std;

class Graph {
    public:
    Graph();
    ~Graph();
    Graph(Logger* logger);
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    int64_t get_last_graph_latency() const;
    Logger* get_logger();
    map<int,vector<int>> get_adjacency_map();
    map<int,Consumer*> get_consumer_container();
    map<int,Node*> get_sink_container();
    map<int,vector<int>> get_update_path();

    template <typename Callback>
    void for_each_producer(Callback&& callback) const {
        for (const auto& producer_entry : this->producer_container) {
            callback(producer_entry.first, static_cast<const Producer*>(producer_entry.second.get()));
        }
    }

    bool empty() const;
    bool has_producers() const;
    int get_node_id(const Node* node) const;
    ///////////////////////////////////// GRAPH CONSTRUCTION LOGIC //////////////////////////


    bool checked_in(const Node* node);

    Market* ensure_market(const string& instrument, const string& exchange);
    void add_edge(Node* publisher,Consumer* subscriber);
    void resolve_output_nodes();
    void resolve_update_path();
    vector<vector<int>> link(int target_node_id);

    ///////////////////////////////////// GRAPH RUNNING LOGIC //////////////////////////

    void update(const int& source_id);
    void ingest_event(Event& event);


private:

    Producer* add_producer(unique_ptr<Producer> source_node);

    int64_t sequence_number;
    int64_t last_in_streamer_timestamp;
    int64_t last_in_order_gateway_timestamp;
    int64_t last_in_graph_timestamp;
    int64_t last_out_graph_timestamp;
    int max_id;

    Logger* logger;

    map<int,vector<int>> adjacency_map;
    map<int,Consumer*> consumer_container;
    map<int,unique_ptr<Producer>> producer_container;
    map<int,Node*> sink_container;
    map<int,vector<int>> update_path;
};



#endif //GRAPH_H
