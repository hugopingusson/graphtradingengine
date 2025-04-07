//
// Created by hugo on 06/04/25.
//

#include "BacktestEngine.h"


BacktestEngine::BacktestEngine():logger(),graph() {}
BacktestEngine::BacktestEngine(const Logger &logger, const Graph &graph):graph(graph),logger(logger),streamer_container() {
}

Graph BacktestEngine::get_graph() {
    return this->graph;
}

Logger BacktestEngine::get_logger() {
    return this->logger;
}

StreamerContainer BacktestEngine::get_streamer_container() {
    return this->streamer_container;
}

void BacktestEngine::build_streamer_container() {
    if (this->graph.get_source_container().empty()) {
        this->logger.log_error("BacktestEngine","Cannot build streamer container, source container is empty");
        throw std::runtime_error("Error in BacktestEngine, cannot build streamer container, source container is empty");
    }else {
        for (auto source : this->graph.get_source_container()) {
            this->streamer_container.register_source(source.second);
        }

    }


}


void BacktestEngine::initialize() {
    this->logger.log_info("BacktestEngine","Initializing Backtest engine");
    this->logger.log_info("BacktestEngine","Resolving update path in graph");
    this->graph.resolve_update_path();
    this->build_streamer_container();




}

