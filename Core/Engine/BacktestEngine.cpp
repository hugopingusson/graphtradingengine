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
    this->streamer_container.route_all_streamers(date);

    std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<>> min_heap;

    this->logger->log_info("BacktestEngine","ENGINE RUNNING");


    for (auto streamer : this->streamer_container.get_streamers()) {
        if (streamer.second->is_good()) {
            min_heap.push(streamer.second->get_current_heap_item());
        }
    }

    while (!min_heap.empty()) {
        HeapItem smallest = min_heap.top();
        min_heap.pop();
        size_t id = smallest.file_id;

        if (typeid(*this->streamer_container.get_streamers()[id])==typeid(DatabaseBacktestStreamer)) {
            DatabaseBacktestStreamer* backtest_streamer = dynamic_cast<DatabaseBacktestStreamer*>(this->streamer_container.get_streamers()[id]);

            if (backtest_streamer->get_current_market_by_price_snapshot().action_data.action!=TRADE & backtest_streamer->get_order_book_source_node_id()>0) {

                OrderBookSnapshotEvent* new_event=new OrderBookSnapshotEvent(backtest_streamer->get_current_market_by_price_snapshot().market_timestamp,
                    0,
                    backtest_streamer->get_order_book_source_node_id(),
                    backtest_streamer->get_current_market_by_price_snapshot().order_book_snapshot_data);


                dynamic_cast<MarketOrderBook*>(this->graph->get_source_container()[backtest_streamer->get_order_book_source_node_id()])->on_event(new_event);

                delete new_event;

                this->graph->update(backtest_streamer->get_order_book_source_node_id());

            }

            if (backtest_streamer->get_current_market_by_price_snapshot().action_data.action!=TRADE & backtest_streamer->get_trade_source_node_id()>0) {
                TradeEvent* new_event = new TradeEvent(backtest_streamer->get_current_market_by_price_snapshot().market_timestamp,
                    0,
                    backtest_streamer->get_trade_source_node_id(),
                    backtest_streamer->get_current_market_by_price_snapshot().action_data.side,
                    backtest_streamer->get_current_market_by_price_snapshot().action_data.price,
                    backtest_streamer->get_current_market_by_price_snapshot().action_data.base_quantity);

                dynamic_cast<MarketTrade*>(this->graph->get_source_container()[backtest_streamer->get_trade_source_node_id()])->on_event(new_event);

                delete new_event;

                this->graph->update(backtest_streamer->get_trade_source_node_id());

            }
        }

        if (this->streamer_container.get_streamers()[id]->advance()) {
            min_heap.push(this->streamer_container.get_streamers()[id]->get_current_heap_item());
        }
    }

}








