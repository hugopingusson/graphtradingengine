//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"

#include <stdexcept>

#include "../../Helper/DataBaseHelper.h"
#include "../Graph/Graph.h"

Streamer::Streamer() {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MarketStreamer::MarketStreamer():order_book_source_node_id() {}
MarketStreamer::MarketStreamer(const string& instrument, const string& exchange):exchange(exchange),instrument(instrument),order_book_source_node_id() {}

string MarketStreamer::get_instrument() const {
    return this->instrument;
}

string MarketStreamer::get_exchange() const {
    return this->exchange;
}

void MarketStreamer::set_order_book_source_node_id(const int& order_book_source_node_id) {
    this->order_book_source_node_id = order_book_source_node_id;
}

int MarketStreamer::get_order_book_source_node_id() const {
    return this->order_book_source_node_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BacktestStreamer::BacktestStreamer():id() {}

void BacktestStreamer::set_id(const size_t& id) {
    this->id = id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DatabaseWMBPBacktestStreamer::DatabaseWMBPBacktestStreamer()
    : file(),
      current_reception_timestamp(),
      current_location(Location::UNKNOWN),
      current_listener(Listener::UNKNOWN),
      current_message() {}
DatabaseWMBPBacktestStreamer::DatabaseWMBPBacktestStreamer(const string& instrument, const string& exchange)
    : MarketStreamer(instrument,exchange),
      BacktestStreamer(),
      file(),
      current_reception_timestamp(),
      current_location(Location::UNKNOWN),
      current_listener(Listener::UNKNOWN),
      current_message() {}

void DatabaseWMBPBacktestStreamer::set_and_route(const Timestamp& start, const Timestamp& /*end*/) {

    DataBaseHelper databe_base_helper = DataBaseHelper();

    string data_path = databe_base_helper.get_data_path(start.get_date().to_string(),this->instrument,this->exchange);
    if (!filesystem::exists(data_path)) {
        throw std::runtime_error(fmt::format("Error when routing streamer {}, bin file {} doesn't exist",this->get_name(),data_path));
    }

    if (this->file.is_open()) {
        this->file.close();
    }
    this->file.clear();
    this->file.open(data_path, std::ios::binary);
    if (!this->file.is_open()) {
        throw std::runtime_error(fmt::format("Error when routing streamer {}, cannot open bin file {}", this->get_name(), data_path));
    }
    int64_t start_ts = start.unixtime();
    if (!this->advance()) {
        return;
    }
    while (this->current_reception_timestamp < start_ts) {
        if (!this->advance()) {
            return;
        }
    }
    // return data_path;

}



string DatabaseWMBPBacktestStreamer::get_name() {
    return fmt::format("DatabaseWMBPBacktestStreamer(instrument={},exchange={})",this->instrument,this->exchange);
}

bool DatabaseWMBPBacktestStreamer::advance() {
    MarketByPriceEventPod row;
    file.read(reinterpret_cast<char*>(&row), sizeof(MarketByPriceEventPod));
    if (file.gcount() != sizeof(MarketByPriceEventPod)) {
        return false;
    }

    this->current_message = row.message;
    this->current_reception_timestamp = row.reception_timestamp;
    this->current_location = row.location;
    this->current_listener = row.listener;
    return true;
}

HeapItem DatabaseWMBPBacktestStreamer::get_current_heap_item() {
    return {this->current_reception_timestamp,this->id};
}

bool DatabaseWMBPBacktestStreamer::is_good() const {
    return this->file.is_open() && this->file.good();
}

void DatabaseWMBPBacktestStreamer::process_current(Graph* graph) {
    MarketByPriceEvent event(
        this->instrument,
        this->current_reception_timestamp,
        this->order_book_source_node_id,
        this->current_message,
        this->current_location,
        this->current_listener
    );

    graph->get_producer_container().find(this->order_book_source_node_id)->second->on_event(&event);
    graph->update(this->order_book_source_node_id);
}

MarketByPriceMessage DatabaseWMBPBacktestStreamer::get_current_message() const {
    return this->current_message;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HeartBeatBackTestStreamer::HeartBeatBackTestStreamer():frequency(),heartbeat_source_node_id(),current_reception_timestamp(),end_reception_timestamp(){}
HeartBeatBackTestStreamer::HeartBeatBackTestStreamer(const double& frequency):frequency(frequency),heartbeat_source_node_id(),current_reception_timestamp(),end_reception_timestamp(){}

string HeartBeatBackTestStreamer::get_name() {
    return fmt::format("HeartBeatBackTestStreamer(frequency={})",std::to_string(frequency));
}

double HeartBeatBackTestStreamer::get_frequency() {
    return this->frequency;
}

void HeartBeatBackTestStreamer::set_heartbeat_source_node_id(const int& heartbeat_source_node_id) {
    this->heartbeat_source_node_id = heartbeat_source_node_id;
}

bool HeartBeatBackTestStreamer::advance() {
    if (this->current_reception_timestamp >= this->end_reception_timestamp) {
        return false;
    }
    this->current_reception_timestamp += int64_t(1e9 * this->frequency);
    return this->current_reception_timestamp <= this->end_reception_timestamp;
}

bool HeartBeatBackTestStreamer::is_good() const {
    return this->current_reception_timestamp <= this->end_reception_timestamp;
}

HeapItem HeartBeatBackTestStreamer::get_current_heap_item() {
    return {this->current_reception_timestamp,this->id};
}


void HeartBeatBackTestStreamer::set_and_route(const Timestamp &start, const Timestamp &end) {
    this->current_reception_timestamp=start.unixtime();
    this->end_reception_timestamp=end.unixtime();
}

void HeartBeatBackTestStreamer::process_current(Graph* graph) {
    if (!graph || this->heartbeat_source_node_id <= 0) {
        return;
    }

    const auto& producers = graph->get_producer_container();
    auto producer_it = producers.find(this->heartbeat_source_node_id);
    if (producer_it == producers.end() || !producer_it->second) {
        return;
    }

    HeartBeatEvent event(
        this->current_reception_timestamp,
        this->heartbeat_source_node_id,
        this->frequency,
        Location::UNKNOWN,
        Listener::PRODUCTION
    );
    producer_it->second->on_event(&event);
    graph->update(this->heartbeat_source_node_id);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackTestStreamerContainer::BackTestStreamerContainer():max_id(),logger(nullptr),streamers(){}
BackTestStreamerContainer::BackTestStreamerContainer(Logger* logger):max_id(),logger(logger),streamers(){}

BackTestStreamerContainer::~BackTestStreamerContainer() {
    if (this->logger) {
        this->logger->log_info("StreamerContainer","Deleting streamers");
    }
    for (auto& streamer : streamers) {
        if (this->logger) {
            this->logger->log_info("StreamerContainer",fmt::format("Deleting streamer: {}",streamer.second->get_name()));
        }
        delete streamer.second;
    }
}

map<size_t,BacktestStreamer*>& BackTestStreamerContainer::get_streamers() {
    return this->streamers;
}

const map<size_t,BacktestStreamer*>& BackTestStreamerContainer::get_streamers() const {
    return this->streamers;
}

void BackTestStreamerContainer::register_source(Producer* source_node) {
    if (!source_node) {
        throw std::runtime_error("Error in register_source: received null source node");
    }

    if (source_node->get_node_id()==0) {
        throw std::runtime_error(fmt::format("Error in register_source,cannot register source node = {}, the node is not attached to a graph",source_node->get_name()));
    }

    if (auto* market_order_book = dynamic_cast<MarketOrderBook*>(source_node)) {
        this->register_market_orderbook_source(market_order_book);
        return;
    }

    if (auto* heartbeat = dynamic_cast<HeartBeat*>(source_node)) {
        this->register_heartbeat_source(heartbeat);
        return;
    }

    throw std::runtime_error(fmt::format("Error in register_source source = {} cannot be registered",source_node->get_name()));
}

void BackTestStreamerContainer::register_heartbeat_source(HeartBeat* heart_beat) {
    HeartBeatBackTestStreamer* new_streamer = new HeartBeatBackTestStreamer(heart_beat->get_frequency());
    if (this->logger) {
        this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
    }
    new_streamer->set_heartbeat_source_node_id(heart_beat->get_node_id());
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    if (this->logger) {
        this->logger->log_info("StreamerContainer",fmt::format("Setting heartbeat source id to {}: Pointing to {} heartbeat_source_node_id={}",new_streamer->get_name(),heart_beat->get_name(),std::to_string(heart_beat->get_node_id())));
    }
    this->streamers[max_id]=new_streamer;
}

void BackTestStreamerContainer::register_market_orderbook_source(MarketOrderBook* market) {
    for (auto& streamer : streamers) {
        auto* market_streamer = dynamic_cast<MarketStreamer*>(streamer.second);
        if (!market_streamer) {
            continue;
        }

        if (market_streamer->get_instrument()==market->get_instrument() && market_streamer->get_exchange()==market->get_exchange()) {
            if (this->logger) {
                this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",streamer.second->get_name(),market->get_name(),std::to_string(market->get_node_id())));
            }
            market_streamer->set_order_book_source_node_id(market->get_node_id());
            return;
        }
    }

    auto* new_streamer = new DatabaseWMBPBacktestStreamer(market->get_instrument(), market->get_exchange());
    if (this->logger) {
        this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
        this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
    }
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    new_streamer->set_order_book_source_node_id(market->get_node_id());
    this->streamers[max_id]=new_streamer;



}

void BackTestStreamerContainer::route_and_set_streamers(const Timestamp &start, const Timestamp &end) {
    for (auto& streamer : this->streamers) {

        streamer.second->set_and_route(start,end);
        // if (this->logger && typeid(streamer)==typeid(DatabaseWMBPBacktestStreamer)) {
        //     this->logger->log_info("StreamerContainer", fmt::format("Routed streamer {} to {}", streamer.second->get_name(), dynamic_cast<DatabaseWMBPBacktestStreamer*>(streamer.second)->get_file().st));
        // }
    }
}
