//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

Graph::Graph() {
    sequence_number=int64_t();
    last_exchange_timestamp=int64_t();
    last_reception_timestamp=int64_t();
    last_in_graph_timestamp=int64_t();
    last_out_graph_timestamp=int64_t();
    adjacency_map=map<int,vector<int>>();
    node_container=map<int,Node*>();
    source_container=map<int,Node*>();
    main_logger=Logger();
};

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


void Graph::checkin(Node* node) {
    if (this->empty()) {
        this->max_id=0;
    }
    else {
        this->max_id+=1;
    }
    node->set_node_id(this->max_id);
    node->set_logger(&this->main_logger);
}


void Graph::add_source(Node* source_node) {

    this->checkin(source_node);
    this->node_container[this->max_id]=source_node;
    this->source_container[this->max_id]=source_node;
    this->adjacency_map[this->max_id]=vector<int>();



}

void Graph::add_edge(Node *publisher, Node *subscriber) {
    int publisher_id = this->get_node_id(publisher);
    if (publisher_id==-1) {
        string error_message=fmt::format("Error in add_egde, could not find publisher node in the Graph: publisher={} , subcriber={}", publisher->get_name(),subscriber->get_name());
        this->main_logger.log_error(error_message);
        throw std::logic_error(error_message);
    }

    this->checkin(subscriber);
    this->node_container[this->max_id]=subscriber;
    this->adjacency_map[publisher_id].push_back(this->max_id);

}


void Graph::add_mono_value_node(MonoValueNode* subscriber) {
    this->add_edge(subscriber->get_parent(),subscriber);
}

void Graph::resolve_update_path() {
    for (auto &it: this->source_container) {
        this->update_path[it.first]=vector<int>();
    }

    for (auto &it: this->source_container) {

    }


}



















