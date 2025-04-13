//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

#include <iostream>
// Graph::Graph():sequence_number(int64_t()),last_order_gateway_in_timestamp(int64_t()),last_streamer_in_timestamp(int64_t()),last_in_graph_timestamp(int64_t()),last_out_graph_timestamp(int64_t()),
//     adjacency_map(map<int,vector<int>>()),child_node_container(map<int,Node*>()),source_container(map<int,Node*>()){
//     logger=Logger("MainLogger","/home/hugo/gte_logs");
// };



Graph::Graph(){
    logger=nullptr;
    sequence_number=int64_t();
    last_in_order_gateway_timestamp=int64_t();
    last_in_streamer_timestamp=int64_t();
    last_in_graph_timestamp=int64_t();
    last_out_graph_timestamp=int64_t();
    adjacency_map=map<int,vector<int>>();
    child_node_container=map<int,ChildNode*>();
    source_container=map<int,SourceNode*>();
    output_container=map<int,ChildNode*>();
    max_id=0;
};


Graph::Graph(Logger *logger) {
    this->logger=logger;
    sequence_number=int64_t();
    last_in_order_gateway_timestamp=int64_t();
    last_in_streamer_timestamp=int64_t();
    last_in_graph_timestamp=int64_t();
    last_out_graph_timestamp=int64_t();
    adjacency_map=map<int,vector<int>>();
    child_node_container=map<int,ChildNode*>();
    source_container=map<int,SourceNode*>();
    output_container=map<int,ChildNode*>();
    max_id=0;
    logger->log_info("Graph","Created Graph");
}



Logger* Graph::get_logger() {
    return this->logger;
}

map<int, vector<int> > Graph::get_adjacency_map() {
    return this->adjacency_map;
}

map<int,ChildNode*> Graph::get_child_node_container() {
    return this->child_node_container;
}

// template <typename Derived>
// map<int,SourceNode<Derived>*> Graph::get_source_container() {
map<int,SourceNode*> Graph::get_source_container() {
    return this->source_container;
}

map<int,ChildNode*> Graph::get_output_container() {
    return this->output_container;
}

map<int,vector<int>> Graph::get_update_path() {
    return this->update_path;
}

int64_t Graph::get_last_graph_latency() const {
    return last_out_graph_timestamp - last_in_graph_timestamp;
}


bool Graph::empty() const {
    return (this->child_node_container.empty() && this->source_container.empty());
}


int Graph::get_node_id(Node* node) const{
    for (auto &it: this->child_node_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    for (auto &it: this->source_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    return -1;
}

///////////////////////////////////// GRAPH CONSTRUCTION LOGIC //////////////////////////

bool Graph::checked_in(Node* node) {
    bool checked_in=false;
    for (auto &it: this->child_node_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }
    for (auto &it: this->source_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }

    return checked_in;

}

// template <typename Derived>
// void Graph::add_source(SourceNode<Derived>* source_node) {
void Graph::add_source(SourceNode* source_node) {
    if (this->empty()) {
        this->max_id=1;
    }
    else {
        this->max_id+=1;
    }
    source_node->set_node_id(this->max_id);
    source_node->set_logger(this->logger);


    this->adjacency_map[this->max_id]=vector<int>();
    this->logger->log_info("Graph",fmt::format("Checked in source node: node name = {} node id = {}",source_node->get_name(),source_node->get_node_id()));

    this->source_container[this->max_id]=source_node;
    this->logger->log_info("Graph",fmt::format("Added source node: {}",source_node->get_name()));
}



void Graph::add_edge(Node *publisher, ChildNode *subscriber) {

    int publisher_id = this->get_node_id(publisher);
    if (publisher_id==-1) {
        string error_message=fmt::format("Error in add_edge, could not find publisher node in the Graph: publisher={} , subcriber={}", publisher->get_name(),subscriber->get_name());
        this->logger->log_error("Graph",error_message);
        throw std::logic_error(error_message);
    }
    if (!this->checked_in(subscriber)) {

        this->max_id+=1;

        subscriber->set_node_id(this->max_id);
        subscriber->set_logger(this->logger);


        this->child_node_container[this->max_id]=subscriber;
        this->adjacency_map[this->max_id]=vector<int>();

        this->logger->log_info("Graph",fmt::format("Checked in child node: node name = {} node id = {}",subscriber->get_name(),subscriber->get_node_id()));
    }
    this->adjacency_map[publisher_id].push_back(this->max_id);

    this->logger->log_info("Graph",fmt::format("Added edge : {} -----> {}",publisher->get_name(),subscriber->get_name()));
}

void Graph::resolve_output_nodes() {
    for (auto &node: this->adjacency_map) {
        if (this->adjacency_map[node.first].empty()) {
            this->output_container[node.first]=this->child_node_container[node.first];
        }
    }
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

void Graph::resolve_update_path() {
    this->resolve_output_nodes();

    vector<vector<int>> all_update_path;
    for (auto &it: this->source_container) {
        for (auto &path : this->link(it.first)){
            all_update_path.push_back(path);
        }
    }

    for (auto &it: this->source_container) {

        vector<vector<int>> sub_path;
        for (auto &path: all_update_path) {
            if (path.front()==it.first) {
                path.erase(path.begin());

                if (path.empty()) {
                    this->logger->log_warn("Graph",fmt::format("Source node = {} is not connected to any child node",it.second->get_name()));
                }else {
                    sub_path.push_back(path);
                }
            }
        }

        while (!sub_path.empty()) {
            vector<int> element_to_erase=vector<int>();

            for (int i=0; i<sub_path.size(); i++) {
                int new_element = sub_path[i][ sub_path[i].size()-1];
                sub_path[i].erase( sub_path[i].end()-1);
                auto p =std::find(this->update_path[it.first].begin(),this->update_path[it.first].end(),new_element);
                if (p==this->update_path[it.first].end()) {
                    this->update_path[it.first].insert(this->update_path[it.first].begin(),new_element);
                }

                if ( sub_path[i].empty()) {
                    element_to_erase.push_back(i);
                }

            }

            for (int i:element_to_erase) {
                sub_path.erase(sub_path.begin()+i);
            }


        }


    }


}






///////////////////////////////////// GRAPH RUNNING LOGIC //////////////////////////


void Graph::update(const int& source_id) {
    for (int& node_id:this->update_path[source_id]) {
        this->child_node_container[node_id]->update();
    }
}










