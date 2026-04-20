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

MarketByPriceBacktestStreamer::MarketByPriceBacktestStreamer()
    : file(),
      current_reception_timestamp(),
      current_location(Location::UNKNOWN),
      current_listener(Listener::UNKNOWN),
      current_message() {}
MarketByPriceBacktestStreamer::MarketByPriceBacktestStreamer(const string& instrument, const string& exchange)
    : MarketStreamer(instrument,exchange),
      BacktestStreamer(),
      file(),
      current_reception_timestamp(),
      current_location(Location::UNKNOWN),
      current_listener(Listener::UNKNOWN),
      current_message() {}

void MarketByPriceBacktestStreamer::set_and_route(const Timestamp& start, const Timestamp& /*end*/) {

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

    {
        std::error_code file_size_ec;
        const uintmax_t bytes = filesystem::file_size(data_path, file_size_ec);
        if (!file_size_ec) {
            constexpr uintmax_t kCurrentRecordSize = static_cast<uintmax_t>(sizeof(MarketByPriceEventPod));
            constexpr uintmax_t kLegacyRecordSize10 = 454; // legacy 10-level MBP pod
            if (bytes > 0 && (bytes % kCurrentRecordSize) != 0) {
                if ((bytes % kLegacyRecordSize10) == 0) {
                    throw std::runtime_error(fmt::format(
                        "Error when routing streamer {}: {} looks like legacy 10-level format (record size {}). "
                        "Current engine expects {} bytes per record (kBookLevels={}). Re-run parquet->bin conversion.",
                        this->get_name(),
                        data_path,
                        kLegacyRecordSize10,
                        kCurrentRecordSize,
                        kBookLevels
                    ));
                }
                throw std::runtime_error(fmt::format(
                    "Error when routing streamer {}: invalid bin size {} for {} (expected multiple of record size {}).",
                    this->get_name(),
                    bytes,
                    data_path,
                    kCurrentRecordSize
                ));
            }
        }
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



string MarketByPriceBacktestStreamer::get_name() {
    return fmt::format("MarketByPriceBacktestStreamer(instrument={},exchange={})",this->instrument,this->exchange);
}

bool MarketByPriceBacktestStreamer::advance() {
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

HeapItem MarketByPriceBacktestStreamer::get_current_heap_item() {
    return {this->current_reception_timestamp,this->id};
}

bool MarketByPriceBacktestStreamer::is_good() const {
    return this->file.is_open() && this->file.good();
}

void MarketByPriceBacktestStreamer::process_current(Graph* graph) {
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

MarketByPriceMessage MarketByPriceBacktestStreamer::get_current_message() const {
    return this->current_message;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    if (auto* market = dynamic_cast<Market*>(source_node)) {
        this->register_market_source(market);
        return;
    }

    throw std::runtime_error(fmt::format("Error in register_source source = {} cannot be registered",source_node->get_name()));
}

void BackTestStreamerContainer::register_market_source(Market* market) {
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

    auto* new_streamer = new MarketByPriceBacktestStreamer(market->get_instrument(), market->get_exchange());
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
        // if (this->logger && typeid(streamer)==typeid(MarketByPriceBacktestStreamer)) {
        //     this->logger->log_info("StreamerContainer", fmt::format("Routed streamer {} to {}", streamer.second->get_name(), dynamic_cast<MarketByPriceBacktestStreamer*>(streamer.second)->get_file().st));
        // }
    }
}
