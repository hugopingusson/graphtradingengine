//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"

#include <math.h>

Streamer::Streamer(){};




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


HeartBeatStreamer::HeartBeatStreamer():frequency(),heartbeat_source_node_id(){};
HeartBeatStreamer::HeartBeatStreamer(const double& frequency):frequency(frequency),heartbeat_source_node_id(){};

double HeartBeatStreamer::get_frequency() {
    return this->frequency;
}

int HeartBeatStreamer::get_target_source_node_id() {
    return this->heartbeat_source_node_id;
}

CmeStreamer::CmeStreamer():MarketStreamer() {}
CmeStreamer::CmeStreamer(const string &instrument, const string &exchange):MarketStreamer(instrument,exchange){};
CmeStreamer::CmeStreamer(const string &instrument, const string &exchange,const int& trade_source_node,const int& order_book_source_node):MarketStreamer(instrument,exchange,trade_source_node,order_book_source_node) {}




StreamerContainer::StreamerContainer(){}
StreamerContainer::~StreamerContainer() {
    for (auto & market_streamer : market_streamers) {
        delete market_streamer;
    }
    for (auto & heartbeat_streamer : heartbeat_streamers) {
        delete heartbeat_streamer;
    }

}



void StreamerContainer::add_cme_streamer(Market *market) {
    for (auto & streamer : market_streamers) {
        if (streamer->get_exchange()=="cme" & streamer->get_instrument()==market->get_instrument()) {
            if (typeid(market)==typeid(MarketTrade)) {
                streamer->set_trade_source_node_id(market->get_node_id());
                return;
            }else if (typeid(market)==typeid(MarketOrderBook)) {
                streamer->set_order_book_source_node_id(market->get_node_id());
                return;
            }
            else {
                throw std::runtime_error(fmt::format("StreamerContainer::register_cme_streamer: unknown market type = {}",market->get_name()));
            }
        }
    }

    CmeStreamer* new_streamer = new CmeStreamer(market->get_instrument(),market->get_exchange());

    if (typeid(market)==typeid(MarketTrade)) {
        new_streamer->set_trade_source_node_id(market->get_node_id());
        this->market_streamers.push_back(new_streamer);
    }else if (typeid(market)==typeid(MarketOrderBook)) {
        new_streamer->set_order_book_source_node_id(market->get_node_id());
        this->market_streamers.push_back(new_streamer);
    }
    else {
        throw std::runtime_error(fmt::format("StreamerContainer::register_cme_streamer: unknown market type = {}",market->get_name()));
    }

}

void StreamerContainer::register_heartbeat_source(HeartBeat* heart_beat) {
    HeartBeatStreamer* new_streamer = new HeartBeatStreamer(heart_beat->get_clock_frequency());
    this->heartbeat_streamers.push_back(new_streamer);
}


void StreamerContainer::register_market_source(Market *market) {
    if (market->get_exchange()=="cme") {
        this->add_cme_streamer(market);
    }else {
        throw std::runtime_error(fmt::format("Error in register_market_source, cannot register exchange={}",market->get_instrument()));
    }
}


void StreamerContainer::register_source(SourceNode* source_node) {
    if (source_node->get_node_id()==0) {
        throw std::runtime_error(fmt::format("Error in register_source,cannot register source node = {}, the node is not attached to a graph",source_node->get_name()));
    }

    // this test doesn't work
    if (typeid(source_node)==typeid(HeartBeat)) {
        this->register_heartbeat_source(dynamic_cast<HeartBeat*>(source_node));
    }
    else if (typeid(source_node)==typeid(MarketOrderBook) | typeid(source_node)==typeid(MarketTrade)) {
        this->register_market_source(dynamic_cast<Market*>(source_node));
    }
    else {
        throw std::runtime_error(fmt::format("Error in register_source source = {} cannot be registered",source_node->get_name()));
    }
}



