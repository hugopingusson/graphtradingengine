//
// Created by hugo on 06/04/25.
//

#include "BacktestEngine.h"


BacktestEngine::BacktestEngine():graph(),logger() {}
BacktestEngine::BacktestEngine(Logger* logger,Graph* graph):graph(graph),logger(logger),streamer_container(BackTestStreamerContainer(logger)){}

Graph* BacktestEngine::get_graph() {
    return this->graph;
}

Logger* BacktestEngine::get_logger() {
    return this->logger;
}

BackTestStreamerContainer BacktestEngine::get_streamer_container() {
    return this->streamer_container;
}

void BacktestEngine::build_streamer_container() {
    if (!this->graph->has_producers()) {
        this->logger->log_error("BacktestEngine","Cannot build streamer container, source container is empty");
        throw std::runtime_error("Error in BacktestEngine, cannot build streamer container, source container is empty");
    }
    this->graph->for_each_producer([this](const int /*source_id*/, const Producer* source) {
        this->streamer_container.register_source(source);
    });




}


void BacktestEngine::initialize() {
    this->logger->log_info("BacktestEngine","Initializing Backtest engine");

    this->logger->log_info("BacktestEngine","Resolving update path in graph");
    this->graph->resolve_update_path();

    this->logger->log_info("BacktestEngine","Building streamers");
    this->build_streamer_container();

    if (this->logger) {
        this->logger->flush();
    }

}




void BacktestEngine::run(const Timestamp& start, const Timestamp& end){
    this->initialize();

    this->logger->log_info("BacktestEngine","Routing streamers");
    this->streamer_container.route_and_set_streamers(start,end);

    std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<>> min_heap;

    this->logger->log_info("BacktestEngine","ENGINE RUNNING");


    auto& streamers = this->streamer_container.get_streamers();
    const int64_t end_timestamp = end.unixtime();

    for (const auto& streamer : streamers) {
        if (streamer.second && streamer.second->is_good()) {
            min_heap.push(streamer.second->get_current_heap_item());
        }
    }
    cout<<"BackestEngine running..."<<endl;
    while (!min_heap.empty()) {
        const HeapItem smallest = min_heap.top();
        if (smallest.row > end_timestamp) {
            break;
        }
        min_heap.pop();
        const size_t id = smallest.file_id;

        // auto streamer_it = streamers.find(id);
        // if (streamer_it == streamers.end() || !streamer_it->second) {
        //     continue;
        // }

        BacktestStreamer* streamer = streamers.find(id)->second;
        streamer->process_current(this->graph);


        if (streamer->advance()) {
            min_heap.push(streamer->get_current_heap_item());
        }
    }

    cout<<"BacktestEngine end"<<endl;
}


