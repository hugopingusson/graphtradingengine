//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

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
    consumer_container=map<int,Consumer*>();
    producer_container=map<int,Producer*>();
    sink_container=map<int,Node*>();
    max_id=0;
}


Graph::Graph(Logger *logger) : Graph() {
    this->logger=logger;
    if (this->logger) {
        this->logger->log_info("Graph","Created Graph");
    }
}



Logger* Graph::get_logger() {
    return this->logger;
}

map<int, vector<int> > Graph::get_adjacency_map() {
    return this->adjacency_map;
}

map<int,Consumer*> Graph::get_consumer_container() {
    return this->consumer_container;
}

// template <typename Derived>
// map<int,SourceNode<Derived>*> Graph::get_source_container() {
const map<int,Producer*>& Graph::get_producer_container() const {
    return this->producer_container;
}

map<int,Node*> Graph::get_sink_container() {
    return this->sink_container;
}

map<int,vector<int>> Graph::get_update_path() {
    return this->update_path;
}

int64_t Graph::get_last_graph_latency() const {
    return last_out_graph_timestamp - last_in_graph_timestamp;
}


bool Graph::empty() const {
    return (this->consumer_container.empty() && this->producer_container.empty());
}


int Graph::get_node_id(Node* node) const{
    for (auto &it: this->consumer_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    for (auto &it: this->producer_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    return -1;
}

///////////////////////////////////// GRAPH CONSTRUCTION LOGIC //////////////////////////

bool Graph::checked_in(Node* node) {
    bool checked_in=false;
    for (auto &it: this->consumer_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }
    for (auto &it: this->producer_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }

    return checked_in;

}

// template <typename Derived>
// void Graph::add_source(SourceNode<Derived>* source_node) {
void Graph::add_producer(Producer* source_node) {
    if (this->empty()) {
        this->max_id=1;
    }
    else {
        this->max_id+=1;
    }
    source_node->set_node_id(this->max_id);
    source_node->set_logger(this->logger);


    this->adjacency_map[this->max_id]=vector<int>();
    if (this->logger) {
        this->logger->log_info("Graph",fmt::format("Checked in source node: node name = {} node id = {}",source_node->get_name(),source_node->get_node_id()));
    }

    this->producer_container[this->max_id]=source_node;
    if (this->logger) {
        this->logger->log_info("Graph",fmt::format("Added source node: {}",source_node->get_name()));
    }
}



void Graph::add_edge(Node *publisher, Consumer *subscriber) {

    int publisher_id = this->get_node_id(publisher);
    if (publisher_id==-1) {
        string error_message=fmt::format("Error in add_edge, could not find publisher node in the Graph: publisher={} , subcriber={}", publisher->get_name(),subscriber->get_name());
        if (this->logger) {
            this->logger->log_error("Graph",error_message);
        }
        throw std::logic_error(error_message);
    }
    int subscriber_id = this->get_node_id(subscriber);
    if (subscriber_id == -1) {
        this->max_id+=1;
        subscriber_id = this->max_id;

        subscriber->set_node_id(subscriber_id);
        subscriber->set_logger(this->logger);


        this->consumer_container[subscriber_id]=subscriber;
        this->adjacency_map[subscriber_id]=vector<int>();

        if (this->logger) {
            this->logger->log_info("Graph",fmt::format("Checked in child node: node name = {} node id = {}",subscriber->get_name(),subscriber->get_node_id()));
        }
    }
    this->adjacency_map[publisher_id].push_back(subscriber_id);

    if (this->logger) {
        this->logger->log_info("Graph",fmt::format("Added edge : {} -----> {}",publisher->get_name(),subscriber->get_name()));
    }
}

void Graph::resolve_output_nodes() {
    for (auto &node: this->adjacency_map) {
        if (this->adjacency_map[node.first].empty()) {
            this->sink_container[node.first]=this->consumer_container[node.first];
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
    for (auto &it: this->producer_container) {
        for (auto &path : this->link(it.first)){
            all_update_path.push_back(path);
        }
    }

    for (auto &it: this->producer_container) {

        vector<vector<int>> sub_path;
        for (auto &path: all_update_path) {
            if (path.front()==it.first) {
                path.erase(path.begin());

                if (path.empty()) {
                    if (this->logger) {
                        this->logger->log_warn("Graph",fmt::format("Source node = {} is not connected to any child node",it.second->get_name()));
                    }
                }else {
                    sub_path.push_back(path);
                }
            }
        }

        while (!sub_path.empty()) {
            vector<int> element_to_erase=vector<int>();

            for (size_t i = 0; i < sub_path.size(); ++i) {
                int new_element = sub_path[i][ sub_path[i].size()-1];
                sub_path[i].erase( sub_path[i].end()-1);
                auto p =std::find(this->update_path[it.first].begin(),this->update_path[it.first].end(),new_element);
                if (p==this->update_path[it.first].end()) {
                    this->update_path[it.first].insert(this->update_path[it.first].begin(),new_element);
                }

                if ( sub_path[i].empty()) {
                    element_to_erase.push_back(static_cast<int>(i));
                }

            }

            for (auto it_idx = element_to_erase.rbegin(); it_idx != element_to_erase.rend(); ++it_idx) {
                sub_path.erase(sub_path.begin() + *it_idx);
            }


        }


    }


}






///////////////////////////////////// GRAPH RUNNING LOGIC //////////////////////////


void Graph::update(const int& source_id) {
    auto source_children_it = this->adjacency_map.find(source_id);
    if (source_children_it != this->adjacency_map.end()) {
        for (const int child_id : source_children_it->second) {
            auto consumer_it = this->consumer_container.find(child_id);
            if (consumer_it != this->consumer_container.end() && consumer_it->second) {
                consumer_it->second->mark_dirty();
            }
        }
    }

    for (int& node_id:this->update_path[source_id]) {
        if (this->consumer_container[node_id]->is_dirty()) {
            if (this->consumer_container[node_id]->update()) {
                for (auto consumer_id: adjacency_map[node_id]) {
                    consumer_container[consumer_id]->mark_dirty();
                }
            }
            this->consumer_container[node_id]->clear_dirty();
        }
    }
}





