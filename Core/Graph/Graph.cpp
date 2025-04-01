//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

#include <iostream>
// Graph::Graph():sequence_number(int64_t()),last_exchange_timestamp(int64_t()),last_reception_timestamp(int64_t()),last_in_graph_timestamp(int64_t()),last_out_graph_timestamp(int64_t()),
//     adjacency_map(map<int,vector<int>>()),node_container(map<int,Node*>()),source_container(map<int,Node*>()){
//     logger=Logger("MainLogger","/home/hugo/gte_logs");
// };



Graph::Graph(){
    logger=Logger("MainLogger","/home/hugo/gte_logs");
    sequence_number=int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    last_in_graph_timestamp=int64_t();
    last_out_graph_timestamp=int64_t();
    adjacency_map=map<int,vector<int>>();
    node_container=map<int,Node*>();
    source_container=map<int,Node*>();
    output_container=map<int,Node*>();
    max_id=0;

    logger.log_info("Created Graph");

};

Logger Graph::get_logger() {
    return this->logger;
}


int64_t Graph::get_last_graph_latency() const {
    return last_out_graph_timestamp - last_in_graph_timestamp;
}

bool Graph::empty() const {
    return this->node_container.empty();
}



int Graph::get_node_id(Node* node) const{
    for (auto &it: this->node_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    return -1;
}

bool Graph::checked_in(Node* node) {
    bool checked_in=false;
    for (auto &it: this->node_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }
    return checked_in;

}

void Graph::checkin(Node* node) {
    if (this->empty()) {
        this->max_id=0;
    }
    else {
        this->max_id+=1;
    }
    node->set_node_id(this->max_id);
    node->set_logger(&this->logger);


    this->node_container[this->max_id]=node;
    this->adjacency_map[this->max_id]=vector<int>();

    this->logger.log_info(fmt::format("Checked in node: node name = {} node id = {}",node->get_name(),node->get_node_id()));
}


void Graph::add_source(Node* source_node) {
    this->checkin(source_node);
    this->source_container[this->max_id]=source_node;
    this->logger.log_info(fmt::format("Added source node: {}",source_node->get_name()));
}

void Graph::add_edge(Node *publisher, Node *subscriber) {

    int publisher_id = this->get_node_id(publisher);
    if (publisher_id==-1) {
        string error_message=fmt::format("Error in add_edge, could not find publisher node in the Graph: publisher={} , subcriber={}", publisher->get_name(),subscriber->get_name());
        this->logger.log_error(error_message);
        throw std::logic_error(error_message);
    }
    if (!this->checked_in(subscriber)) {
        this->checkin(subscriber);
    }
    this->adjacency_map[publisher_id].push_back(this->max_id);

    this->logger.log_info(fmt::format("Added edge : {} -----> {}",publisher->get_name(),subscriber->get_name()));
}

void Graph::resolve_output_nodes() {
    for (auto &node: this->adjacency_map) {
        if (this->adjacency_map[node.first].empty()) {
            this->output_container[node.first]=this->node_container[node.first];
        }
    }
}

// void Graph::add_mono_value_node(MonoValueNode* subscriber) {
//     this->add_edge(subscriber->get_parent(),subscriber);
// }

void Graph::resolve_update_path() {
    this->resolve_output_nodes();

    vector<vector<int>> all_update_path;
    for (auto &it: this->source_container) {
        for (auto &path : this->link(it.first)){
            all_update_path.push_back(path);
        }
    }

    for (auto &it: this->output_container) {
        vector<vector<int>> output_element;
        for (auto &path: all_update_path) {
        }
    }

   cout << all_update_path.size() << endl;

}


vector<vector<int>> Graph::link(int target_node_id) {
    vector<vector<int>> return_vector;
    if (this->adjacency_map[target_node_id].empty()) {
        return_vector.push_back(vector<int>({target_node_id}));
    }
    else {
        for (auto it: this->adjacency_map[target_node_id]) {
            for (auto incoming_vector :this->link(it)){
                incoming_vector.insert(incoming_vector.begin(),target_node_id);
                return_vector.push_back(incoming_vector);
            }
        }
    }
    return return_vector;


}

















