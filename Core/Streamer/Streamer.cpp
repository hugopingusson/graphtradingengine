//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"

#include <stdexcept>

#include "../../Helper/DataBaseHelper.h"

namespace {
#pragma pack(push, 1)
struct BacktestMarketRow {
    MarketTimeStamp market_time_stamp{};
    Action action{Action::ADD};
    Side side{Side::NEUTRAL};
    double price{0.0};
    double base_quantity{0.0};
    SnapshotData order_book_snapshot_data{};
};
#pragma pack(pop)
}


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

DatabaseBacktestStreamer::DatabaseBacktestStreamer():
    file(),
    current_market_by_price_message(),
    current_action(Action::ADD),
    current_side(Side::NEUTRAL),
    current_price(0.0),
    current_base_quantity(0.0) {}

DatabaseBacktestStreamer::DatabaseBacktestStreamer(const string &instrument, const string &exchange):
    MarketStreamer(instrument,exchange),
    current_market_by_price_message(),
    current_action(Action::ADD),
    current_side(Side::NEUTRAL),
    current_price(0.0),
    current_base_quantity(0.0) {}

string DatabaseBacktestStreamer::route_streamer(const std::string& date) {
    DataBaseHelper databe_base_helper = DataBaseHelper();
    string data_path = databe_base_helper.get_data_path(date,this->instrument,this->exchange);
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
    this->advance();
    return data_path;
}

bool DatabaseBacktestStreamer::advance() {
    BacktestMarketRow row;
    file.read(reinterpret_cast<char*>(&row), sizeof(BacktestMarketRow));
    if (file.gcount() != sizeof(BacktestMarketRow)) {
        return false;
    }

    this->current_market_by_price_message.instrument = this->instrument;
    this->current_market_by_price_message.market_time_stamp = row.market_time_stamp;
    this->current_market_by_price_message.order_book_snapshot_data = row.order_book_snapshot_data;

    this->current_action = row.action;
    this->current_side = row.side;
    this->current_price = row.price;
    this->current_base_quantity = row.base_quantity;
    return true;
}


bool DatabaseBacktestStreamer::is_good() const {
    return this->file.is_open() && this->file.good();
}

string DatabaseBacktestStreamer::get_name() {
    return fmt::format("DatabaseBacktestStreamer(instrument={},exchange={})",this->instrument,this->exchange);
}

HeapItem DatabaseBacktestStreamer::get_current_heap_item() {
    return {this->current_market_by_price_message.market_time_stamp,this->id};
}


MarketTimeStamp DatabaseBacktestStreamer::get_current_market_timestamp() {
    return this->current_market_by_price_message.market_time_stamp;
}

MarketByPriceMessage DatabaseBacktestStreamer::get_current_market_by_price_message() {
    return this->current_market_by_price_message;
}

MarketByPriceMessage DatabaseBacktestStreamer::get_current_market_by_price_snapshot() {
    return this->current_market_by_price_message;
}

Action DatabaseBacktestStreamer::get_current_action() const {
    return this->current_action;
}

Side DatabaseBacktestStreamer::get_current_side() const {
    return this->current_side;
}

double DatabaseBacktestStreamer::get_current_price() const {
    return this->current_price;
}

double DatabaseBacktestStreamer::get_current_base_quantity() const {
    return this->current_base_quantity;
}

Event* DatabaseBacktestStreamer::materialize_event() {
    if (this->current_action == Action::TRADE) {
        if (this->trade_source_node_id <= 0) {
            return nullptr;
        }
        return new TradeEvent(
            this->current_market_by_price_message.market_time_stamp,
            0,
            this->trade_source_node_id,
            this->current_side,
            this->current_price,
            this->current_base_quantity
        );
    }

    if (this->order_book_source_node_id <= 0) {
        return nullptr;
    }

    return new MBPEvent(
        this->current_market_by_price_message.market_time_stamp,
        0,
        this->order_book_source_node_id,
        this->current_market_by_price_message
    );
}

int DatabaseBacktestStreamer::get_target_source_node_id() const {
    if (this->current_action == Action::TRADE) {
        return this->trade_source_node_id;
    }
    return this->order_book_source_node_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HeartBeatBackTestStreamer::HeartBeatBackTestStreamer():frequency(),heartbeat_source_node_id(),emitted_once(false),current_market_timestamp(){};
HeartBeatBackTestStreamer::HeartBeatBackTestStreamer(const double& frequency):frequency(frequency),heartbeat_source_node_id(),emitted_once(false),current_market_timestamp(){};

string HeartBeatBackTestStreamer::get_name() {
    return fmt::format("HeartBeatBackTestStreamer(frequency={})",std::to_string(frequency));
}

double HeartBeatBackTestStreamer::get_frequency() {
    return this->frequency;
}

int HeartBeatBackTestStreamer::get_target_source_node_id() const {
    return this->heartbeat_source_node_id;
}

void HeartBeatBackTestStreamer::set_heartbeat_source_node_id(const int &heartbeat_source_node_id) {
    this->heartbeat_source_node_id = heartbeat_source_node_id;
}

bool HeartBeatBackTestStreamer::advance() {
    if (this->emitted_once) {
        return false;
    }
    this->emitted_once = true;
    return false;
}

bool HeartBeatBackTestStreamer::is_good() const {
    return !this->emitted_once;
}

HeapItem HeartBeatBackTestStreamer::get_current_heap_item() {
    return {this->current_market_timestamp,this->id};
}

Event* HeartBeatBackTestStreamer::materialize_event() {
    if (this->heartbeat_source_node_id <= 0) {
        return nullptr;
    }
    return new HeartBeatEvent(
        this->current_market_timestamp.capture_server_in_timestamp,
        this->heartbeat_source_node_id,
        this->frequency
    );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BackTestStreamerContainer::BackTestStreamerContainer():logger(nullptr),max_id(),streamers(){}
BackTestStreamerContainer::BackTestStreamerContainer(Logger* logger):logger(logger),max_id(),streamers(){}

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
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {

            if (dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_instrument()==market->get_instrument() && dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_exchange()==market->get_exchange()) {
                if (this->logger) {
                    this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",streamer.second->get_name(),market->get_name(),std::to_string(market->get_node_id())));
                }
                dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->set_order_book_source_node_id(market->get_node_id());
                return;
            }
        }
    }

    DatabaseBacktestStreamer* new_streamer = new DatabaseBacktestStreamer(market->get_instrument(),market->get_exchange());
    if (this->logger) {
        this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
        this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
    }
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    new_streamer->set_order_book_source_node_id(market->get_node_id());
    this->streamers[max_id]=new_streamer;


}

void BackTestStreamerContainer::register_market_trade_source(const string& instrument, const string& exchange, const int& node_id) {
    for (auto& streamer : streamers) {
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {

            if (dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_instrument()==instrument && dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->get_exchange()==exchange) {
                if (this->logger) {
                    this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: trade_source_node_id={}",streamer.second->get_name(),std::to_string(node_id)));
                }
                dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->set_trade_source_node_id(node_id);
                return;
            }
        }
    }

    DatabaseBacktestStreamer* new_streamer = new DatabaseBacktestStreamer(instrument,exchange);
    if (this->logger) {
        this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
        this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: trade_source_node_id={}",new_streamer->get_name(),std::to_string(node_id)));
    }
    this->max_id+=1;
    new_streamer->set_id(this->max_id);
    new_streamer->set_trade_source_node_id(node_id);
    this->streamers[max_id]=new_streamer;


}



void BackTestStreamerContainer::route_all_streamers(const string &date) {
    for (auto& streamer : this->streamers) {
        if (typeid(*streamer.second)==typeid(DatabaseBacktestStreamer)) {
            string path = dynamic_cast<DatabaseBacktestStreamer*>(streamer.second)->route_streamer(date);
            if (this->logger) {
                this->logger->log_info("StreamerContainer", fmt::format("Routed streamer {} to {}", streamer.second->get_name(), path));
            }
        }
    }
}
