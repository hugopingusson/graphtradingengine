//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"

#include <math.h>


Streamer::Streamer(){};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MarketStreamer::MarketStreamer():trade_source_node_id(),order_book_source_node_id(){}
MarketStreamer::MarketStreamer(const string &instrument, const string &exchange):trade_source_node_id(),order_book_source_node_id(),exchange(exchange),instrument(instrument) {}
MarketStreamer::MarketStreamer(const string &instrument, const string &exchange,const int& trade_source_node_id,const int& order_book_source_node_id):exchange(exchange),instrument(instrument),trade_source_node_id(trade_source_node_id),order_book_source_node_id(order_book_source_node_id) {}

string MarketStreamer::get_instrument() {
    return this->instrument;
}

string MarketStreamer::get_exchange() {
    return this->exchange;
}

int MarketStreamer::get_order_book_source_node_id() {
    return this->order_book_source_node_id;
}

int MarketStreamer::get_trade_source_node_id() {
    return this->trade_source_node_id;
}

void MarketStreamer::set_order_book_source_node_id(const int& order_book_source_node_id) {
    this->order_book_source_node_id = order_book_source_node_id;
}


void MarketStreamer::set_trade_source_node_id(const int& trade_source_node_id) {
    this->trade_source_node_id = trade_source_node_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HeartBeatStreamer::HeartBeatStreamer():frequency(),heartbeat_source_node_id(){};
HeartBeatStreamer::HeartBeatStreamer(const double& frequency):frequency(frequency),heartbeat_source_node_id(){};

string HeartBeatStreamer::get_name() {
    return fmt::format("HeartBeatStreamer(frequency={})",std::to_string(frequency));
}

double HeartBeatStreamer::get_frequency() {
    return this->frequency;
}

int HeartBeatStreamer::get_target_source_node_id() {
    return this->heartbeat_source_node_id;
}

void HeartBeatStreamer::set_heartbeat_source_node_id(const int &heartbeat_source_node_id) {
    this->heartbeat_source_node_id = heartbeat_source_node_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CmeStreamer::CmeStreamer():MarketStreamer() {}
CmeStreamer::CmeStreamer(const string &instrument, const string &exchange):MarketStreamer(instrument,exchange){};
CmeStreamer::CmeStreamer(const string &instrument, const string &exchange,const int& trade_source_node,const int& order_book_source_node):MarketStreamer(instrument,exchange,trade_source_node,order_book_source_node) {}

string CmeStreamer::get_name() {
    return fmt::format("CmeStreamer(instrument={},exchange={})",this->instrument,this->exchange);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


StreamerContainer::StreamerContainer():logger(nullptr){}
StreamerContainer::StreamerContainer(Logger* logger):logger(logger){}

StreamerContainer::~StreamerContainer() {
    this->logger->log_info("StreamerContainer","Deleting streamers starts");

    for (auto& market_streamer : market_streamers) {
        this->logger->log_info("StreamerContainer",fmt::format("Deleting market streamer: {}",market_streamer->get_name()));
        delete market_streamer;
    }
    for (auto& heartbeat_streamer : heartbeat_streamers) {
        this->logger->log_info("StreamerContainer",fmt::format("Deleting heartbeat streamer: {}",heartbeat_streamer->get_name()));
        delete heartbeat_streamer;
    }
}



void StreamerContainer::add_cme_streamer(Market* market) {
    for (auto & streamer : market_streamers) {
        if (streamer->get_exchange()=="cme" & streamer->get_instrument()==market->get_instrument()) {
            if (typeid(*market)==typeid(MarketTrade)) {
                this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: Pointing to {} trade_source_id={}",streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
                streamer->set_trade_source_node_id(market->get_node_id());
                return;
            }else if (typeid(*market)==typeid(MarketOrderBook)) {
                this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
                streamer->set_order_book_source_node_id(market->get_node_id());
                return;
            }
            else {
                throw std::runtime_error(fmt::format("StreamerContainer::register_cme_streamer: unknown market type = {}",market->get_name()));
            }
        }
    }


    CmeStreamer* new_streamer = new CmeStreamer(market->get_instrument(),market->get_exchange());
    this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));

    if (typeid(*market)==typeid(MarketTrade)) {
        this->logger->log_info("StreamerContainer",fmt::format("Setting trade source id to {}: Pointing to {} trade_source_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
        new_streamer->set_trade_source_node_id(market->get_node_id());
        this->market_streamers.push_back(new_streamer);
    }else if (typeid(*market)==typeid(MarketOrderBook)) {
        this->logger->log_info("StreamerContainer",fmt::format("Setting order book source id to {}: Pointing to {} order_book_source_node_id={}",new_streamer->get_name(),market->get_name(),std::to_string(market->get_node_id())));
        new_streamer->set_order_book_source_node_id(market->get_node_id());
        this->market_streamers.push_back(new_streamer);
    }
    else {
        throw std::runtime_error(fmt::format("StreamerContainer::register_cme_streamer: unknown market type = {}",market->get_name()));
    }

}

void StreamerContainer::register_heartbeat_source(HeartBeat* heart_beat) {
    HeartBeatStreamer* new_streamer = new HeartBeatStreamer(heart_beat->get_frequency());
    this->logger->log_info("StreamerContainer",fmt::format("Creating new streamer : {}",new_streamer->get_name()));
    new_streamer->set_heartbeat_source_node_id(heart_beat->get_node_id());
    this->logger->log_info("StreamerContainer",fmt::format("Setting heartbeat source id to {}: Pointing to {} heartbeat_source_node_id={}",new_streamer->get_name(),heart_beat->get_name(),std::to_string(heart_beat->get_node_id())));
    this->heartbeat_streamers.push_back(new_streamer);
}


void StreamerContainer::register_market_source(Market *market) {
    if (market->get_exchange()=="cme") {
        this->add_cme_streamer(market);
    }else {
        throw std::runtime_error(fmt::format("Error in register_market_source, cannot register exchange={}",market->get_instrument()));
    }
}

// template <typename Derived>
// void StreamerContainer::register_source(SourceNode<Derived>* source_node) {
void StreamerContainer::register_source(SourceNode* source_node) {
    if (source_node->get_node_id()==0) {
        throw std::runtime_error(fmt::format("Error in register_source,cannot register source node = {}, the node is not attached to a graph",source_node->get_name()));
    }

    // this test doesn't work
    if (typeid(*source_node)==typeid(HeartBeat)) {
        this->register_heartbeat_source(dynamic_cast<HeartBeat*>(source_node));
    }
    else if (typeid(*source_node)==typeid(MarketOrderBook) | typeid(*source_node)==typeid(MarketTrade)) {
        this->register_market_source(dynamic_cast<Market*>(source_node));
    }
    else {
        throw std::runtime_error(fmt::format("Error in register_source source = {} cannot be registered",source_node->get_name()));
    }
}



