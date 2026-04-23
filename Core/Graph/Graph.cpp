//
// Created by hugo on 25/03/25.
//

#include "Graph.h"

#include <algorithm>
#include <stdexcept>

#include "../../Helper/SaphirManager.h"

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
    producer_container=map<int,unique_ptr<Producer>>();
    sink_container=map<int,Node*>();
    max_id=0;
}


Graph::Graph(Logger *logger) : Graph() {
    this->logger=logger;
    if (this->logger) {
        this->logger->log_info("Graph","Created Graph");
    }
}

Graph::~Graph() = default;



Logger* Graph::get_logger() {
    return this->logger;
}

map<int, vector<int> > Graph::get_adjacency_map() {
    return this->adjacency_map;
}

map<int,Consumer*> Graph::get_consumer_container() {
    return this->consumer_container;
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


bool Graph::has_producers() const {
    return !this->producer_container.empty();
}


int Graph::get_node_id(const Node* node) const{
    for (auto &it: this->consumer_container) {
        if (it.second==node) {
            return it.first;
        }
    }
    for (auto &it: this->producer_container) {
        if (it.second.get()==node) {
            return it.first;
        }
    }
    return -1;
}

///////////////////////////////////// GRAPH CONSTRUCTION LOGIC //////////////////////////

bool Graph::checked_in(const Node* node) {
    bool checked_in=false;
    for (auto &it: this->consumer_container) {
        if (it.second==node) {
            checked_in=true;
        }
    }
    for (auto &it: this->producer_container) {
        if (it.second.get()==node) {
            checked_in=true;
        }
    }

    return checked_in;

}

Producer* Graph::add_producer(unique_ptr<Producer> source_node) {
    if (!source_node) {
        throw std::runtime_error("Graph::add_producer received null producer");
    }

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

    Producer* source_node_ptr = source_node.get();
    this->producer_container[this->max_id]=std::move(source_node);
    if (this->logger) {
        this->logger->log_info("Graph",fmt::format("Added source node: {}",source_node_ptr->get_name()));
    }
    return source_node_ptr;
}


Market* Graph::ensure_market(const string& instrument, const string& exchange) {
    for (const auto& producer_entry : this->producer_container) {
        auto* market = dynamic_cast<Market*>(producer_entry.second.get());
        if (!market) {
            continue;
        }
        if (market->get_instrument() != instrument) {
            continue;
        }
        if (!exchange.empty() && market->get_exchange() != exchange) {
            continue;
        }
        return market;
    }

    if (exchange.empty()) {
        throw std::runtime_error(
            "Graph::ensure_market cannot create market without exchange"
        );
    }

    const SaphirManager saphir;
    const double tick_value = saphir.get_market_tick_value(exchange, instrument);
    auto market = std::make_unique<Market>(
        instrument,
        exchange,
        static_cast<int>(kBookLevels),
        tick_value
    );
    return dynamic_cast<Market*>(this->add_producer(std::move(market)));
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
            auto consumer_it = this->consumer_container.find(node.first);
            if (consumer_it != this->consumer_container.end()) {
                this->sink_container[node.first]=consumer_it->second;
            }
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
    for (const int child_id : this->adjacency_map[source_id]) {
        this->consumer_container[child_id]->mark_dirty();
    }

    for (const int node_id : this->update_path[source_id]) {
        Consumer* consumer = this->consumer_container[node_id];
        if (consumer->is_dirty()) {
            if (consumer->update()) {
                for (const int consumer_id : this->adjacency_map[node_id]) {
                    this->consumer_container[consumer_id]->mark_dirty();
                }
            }
            consumer->clear_dirty();
        }
    }
}

void Graph::ingest_event(Event& event) {
    const int source_id = event.get_source_id_trigger();
    auto producer_it = this->producer_container.find(source_id);
    if (producer_it == this->producer_container.end()) {
        throw std::runtime_error(fmt::format(
            "Graph::ingest_event unknown source_id_trigger={} for event",
            source_id
        ));
    }

    producer_it->second->on_event(&event);
    this->update(source_id);
}
