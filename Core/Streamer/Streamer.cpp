//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"

#include <math.h>

#include "../../Helper/DataBaseHelper.h"


Streamer::Streamer(){};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MarketStreamer::MarketStreamer(){}
MarketStreamer::MarketStreamer(const string &instrument, const string &exchange):exchange(exchange),instrument(instrument) {}

string MarketStreamer::get_instrument() {
    return this->instrument;
}

string MarketStreamer::get_exchange() {
    return this->exchange;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OrderBookStreamer::OrderBookStreamer():order_book_source_node_id() {}
// OrderBookStreamer::OrderBookStreamer(const string &instrument, const string &exchange):MarketStreamer(instrument,exchange),order_book_source_node_id() {}


int OrderBookStreamer::get_order_book_source_node_id() {
    return this->order_book_source_node_id;
}
void OrderBookStreamer::set_order_book_source_node_id(const int& order_book_source_node_id) {
    this->order_book_source_node_id = order_book_source_node_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TradeStreamer::TradeStreamer():trade_source_node_id() {}
// TradeStreamer::TradeStreamer(const string &instrument, const string &exchange):MarketStreamer(instrument,exchange),trade_source_node_id() {}


int TradeStreamer::get_trade_source_node_id() {
    return this->trade_source_node_id;
}

void TradeStreamer::set_trade_source_node_id(const int& trade_source_node_id) {
    this->trade_source_node_id = trade_source_node_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BacktestStreamer::BacktestStreamer():id(){}

void BacktestStreamer::set_id(const size_t &id) {
    this->id = id;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DatabaseBacktestStreamer::DatabaseBacktestStreamer():file(),current_market_by_price_snapshot() {}
DatabaseBacktestStreamer::DatabaseBacktestStreamer(const string &instrument, const string &exchange):MarketStreamer(instrument,exchange),current_market_by_price_snapshot() {}

string DatabaseBacktestStreamer::route_streamer(const std::string& date) {
    DataBaseHelper databe_base_helper = DataBaseHelper();
    string data_path = databe_base_helper.get_data_path(date,this->instrument,this->exchange);
    file.open(data_path, std::ios::binary);
    advance();
    return data_path;
}

bool DatabaseBacktestStreamer::advance() {
    file.read(reinterpret_cast<char*>(&current_market_by_price_snapshot), sizeof(MarketByPriceSnapshot));
    return file.gcount() == sizeof(MarketByPriceSnapshot);
}


bool DatabaseBacktestStreamer::is_good() const {
    return file.good();
}

string DatabaseBacktestStreamer::get_name() {
    return fmt::format("DatabaseBacktestStreamer(instrument={},exchange={})",this->instrument,this->exchange);
}

HeapItem DatabaseBacktestStreamer::get_current_heap_item() {
    return {this->current_market_by_price_snapshot.market_time_stamp,this->id};
}


MarketTimeStamp DatabaseBacktestStreamer::get_current_market_timestamp() {
    return this->current_market_by_price_snapshot.market_time_stamp;
}

MarketByPriceSnapshot DatabaseBacktestStreamer::get_current_market_by_price_snapshot() {
    return this->current_market_by_price_snapshot;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HeartBeatBackTestStreamer::HeartBeatBackTestStreamer():frequency(),heartbeat_source_node_id(),current_market_timestamp(){};
HeartBeatBackTestStreamer::HeartBeatBackTestStreamer(const double& frequency):frequency(frequency),heartbeat_source_node_id(),current_market_timestamp(){};

string HeartBeatBackTestStreamer::get_name() {
    return fmt::format("HeartBeatBackTestStreamer(frequency={})",std::to_string(frequency));
}

double HeartBeatBackTestStreamer::get_frequency() {
    return this->frequency;
}

int HeartBeatBackTestStreamer::get_target_source_node_id() {
    return this->heartbeat_source_node_id;
}

void HeartBeatBackTestStreamer::set_heartbeat_source_node_id(const int &heartbeat_source_node_id) {
    this->heartbeat_source_node_id = heartbeat_source_node_id;
}

bool HeartBeatBackTestStreamer::advance() {
    return true;
}

bool HeartBeatBackTestStreamer::is_good() const {
    return true;
}

HeapItem HeartBeatBackTestStreamer::get_current_heap_item() {
    return {this->current_market_timestamp,this->id};
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BackTestStreamerContainer::BackTestStreamerContainer():logger(nullptr),max_id(),streamers(){}
BackTestStreamerContainer::BackTestStreamerContainer(Logger* logger):logger(logger),max_id(),streamers(){}

BackTestStreamerContainer::~BackTestStreamerContainer() {
    this->logger->log_info("StreamerContainer","Deleting streamers");
    for (auto& streamer : streamers) {
        this->logger->log_info("StreamerContainer",fmt::format("Deleting order book streamer: {}",streamer.second->get_name()));
        delete streamer.second;
    }
}

map<size_t,BacktestStreamer*> BackTestStreamerContainer::get_streamers() {
    return this->streamers;
}

void BackTestStreamerContainer::register_source(SourceNode* source_node) {
    if (source_node->get_node_id()==0) {
        throw std::runtime_error(fmt::format("Error in register_source,cannot register source node = {}, the node is not attached to a graph",source_node->get_name()));
    }

    if (typeid(*source_node)==typeid(MarketOrderBook)) {
        this->register_market_orderbook_source(dynamic_cast<MarketOrderBook *>(source_node));
    }else if (typeid(*source_node)==typeid(MarketTrade)) {
        this->register_market_trade_source(dynamic_cast<MarketTrade *>(source_node));
    }else if (typeid(*source_node)==typeid(HeartBeat)) {
        this->register_heartbeat_source(dynamic_cast<HeartBeat*>(source_node));
    }
    else {
        throw std::runtime_error(fmt::format("Error in register_source source = {} cannot be registered",source_node->get_name()));
    }
}




void BackTestStreamerContainer::register_heartbeat_source(HeartBeat* heart_beat) {
    HeartBeatBackTestStreamer* new_streamer = new HeartBeatBackTestStreamer(heart_beat->get_frequency());
    this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
    new_streamer->set_heartbeat_source_node_id(heart_beat->get_node_id());
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    this->logger->log_info("StreamerContainer",fmt::format("Setting heartbeat source id to {}: Pointing to {} heartbeat_source_node_id={}",new_streamer->get_name(),heart_beat->get_name(),std::to_string(heart_beat->get_node_id())));
    this->streamers[max_id]=new_streamer;
}



void BackTestStreamerContainer::register_market_orderbook_source(MarketOrderBook* market) {
    for (auto& streamer : streamers) {
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {

            if (dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_instrument()==market->get_instrument() & dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_exchange()==market->get_exchange()) {
                this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",streamer.second->get_name(),market->get_name(),std::to_string(market->get_node_id())));
                dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->set_order_book_source_node_id(market->get_node_id());
                return;
            }
        }
    }

    DatabaseBacktestStreamer* new_streamer = new DatabaseBacktestStreamer(market->get_instrument(),market->get_exchange());
    this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
    this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    new_streamer->set_order_book_source_node_id(market->get_node_id());
    this->streamers[max_id]=new_streamer;


}

void BackTestStreamerContainer::register_market_trade_source(MarketTrade* market) {
    for (auto& streamer : streamers) {
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {

            if (dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_instrument()==market->get_instrument() & dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_exchange()==market->get_exchange()) {
                this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: Pointing to {} trade_source_node_id={}",streamer.second->get_name(),market->get_name(),std::to_string(market->get_node_id())));
                dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->set_trade_source_node_id(market->get_node_id());
                return;
            }
        }
    }

    DatabaseBacktestStreamer* new_streamer = new DatabaseBacktestStreamer(market->get_instrument(),market->get_exchange());
    this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
    this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: Pointing to {} trade_source_node_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    new_streamer->set_trade_source_node_id(market->get_node_id());
    this->streamers[max_id]=new_streamer;


}



void BackTestStreamerContainer::route_all_streamers(const string &date) {
    std::vector<std::string> all_path;
    for (auto& streamer : this->streamers) {
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {
            all_path.push_back(dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->route_streamer(date));
        }
    }
}


