//
// Created by hugo on 06/04/25.
//

#include "BacktestEngine.h"


BacktestEngine::BacktestEngine():logger(),graph() {}
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
    if (this->graph->get_source_container().empty()) {
        this->logger->log_error("BacktestEngine","Cannot build streamer container, source container is empty");
        throw std::runtime_error("Error in BacktestEngine, cannot build streamer container, source container is empty");
    }
    for (auto source : this->graph->get_source_container()) {
        this->streamer_container.register_source(source.second);
    }




}


void BacktestEngine::initialize() {
    this->logger->log_info("BacktestEngine","Initializing Backtest engine");

    this->logger->log_info("BacktestEngine","Resolving update path in graph");
    this->graph->resolve_update_path();

    this->logger->log_info("BacktestEngine","Building streamers");
    this->build_streamer_container();


}




void BacktestEngine::run(const string& date) {

    this->logger->log_info("BacktestEngine","Routing streamers");
    const std::vector<std::string>& file_paths = this->streamer_container.route_all_streamers(date);

    // std::vector<StreamCursor> sources;
    std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<>> min_heap;

    this->logger->log_info("BacktestEngine","ENGINE RUNNING");

    // // Open all files and load the first row from each
    // for (size_t i = 0; i < file_paths.size(); ++i) {
    //     sources.emplace_back(file_paths[i], i);
    //     if (sources.back().is_good()) {
    //         min_heap.push({ sources.back().current, i });
    //     }
    // }
    //
    // // std::ofstream output(output_path, std::ios::binary);
    // // if (!output.is_open()) {
    // //     throw std::runtime_error("Failed to open output file: " + output_path);
    // // }
    //
    // while (!min_heap.empty()) {
    //     HeapItem smallest = min_heap.top();
    //     min_heap.pop();
    //
    //     // Do something
    //
    //     // Advance the corresponding stream and reinsert
    //     size_t id = smallest.file_id;
    //     if (sources[id].advance()) {
    //         min_heap.push({ sources[id].current, id });
    //     }
    // }

    // output.close();
}



// void BacktestEngine::run(const string& date) {
//
//
//     const std::vector<std::string>& file_paths = this->streamer_container.route_all_streamers(date);
//
//     // std::vector<StreamCursor> sources;
//     std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<>> min_heap;
//
//     // Open all files and load the first row from each
//     for (size_t i = 0; i < file_paths.size(); ++i) {
//         sources.emplace_back(file_paths[i], i);
//         if (sources.back().is_good()) {
//             min_heap.push({ sources.back().current, i });
//         }
//     }
//
//     // std::ofstream output(output_path, std::ios::binary);
//     // if (!output.is_open()) {
//     //     throw std::runtime_error("Failed to open output file: " + output_path);
//     // }
//
//     while (!min_heap.empty()) {
//         HeapItem smallest = min_heap.top();
//         min_heap.pop();
//
//         // Write the smallest OrderBookSnapshot to output
//         output.write(reinterpret_cast<const char*>(&smallest.row), sizeof(OrderBookSnapshot));
//
//         // Advance the corresponding stream and reinsert
//         size_t id = smallest.file_id;
//         if (sources[id].advance()) {
//             min_heap.push({ sources[id].current, id });
//         }
//     }
//
//     // output.close();
// }
//







