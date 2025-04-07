//
// Created by hugo on 25/03/25.
//

#include "Event.h"




Event::Event():last_streamer_in_timestamp(int64_t()),source_id_trigger(){};
Event::Event(const int64_t &last_streamer_in_timestamp,const int& source_id_trigger):last_streamer_in_timestamp(last_streamer_in_timestamp),source_id_trigger(source_id_trigger) {};

int64_t Event::get_last_streamer_in_timestamp() const {
  return this->last_streamer_in_timestamp;
}

int Event::get_source_id_trigger() const {
    return this->source_id_trigger;
}


HeartBeat::HeartBeat():Event(){};
HeartBeat::HeartBeat(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency):Event(last_streamer_in_timestamp,source_id_trigger),clock_frequency(clock_frequency){};


double HeartBeat::get_clock_frequency() const {
    return this->clock_frequency;
}



MarketEvent::MarketEvent():Event(),last_order_gateway_in_timestamp(int64_t()),last_capture_server_in_timestamp(int64_t()) {};
MarketEvent::MarketEvent(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger):Event(last_streamer_in_timestamp,source_id_trigger),last_order_gateway_in_timestamp(last_order_gateway_in_timestamp),last_capture_server_in_timestamp(last_capture_server_in_timestamp) {};

int64_t MarketEvent::get_last_order_gateway_in_timestamp() const {
    return this->last_order_gateway_in_timestamp;
}
int64_t MarketEvent::get_last_capture_server_in_timestamp() const {
    return this->last_capture_server_in_timestamp;
}


OrderBookSnapshot::OrderBookSnapshot():MarketEvent() {}
OrderBookSnapshot::OrderBookSnapshot(const int64_t &last_order_gateway_in_timestamp, const int64_t &last_capture_server_in_timestamp, const int64_t &last_streamer_in_timestamp,const int& source_id_trigger,const map<string,vector<double>>& data) {
    this->last_order_gateway_in_timestamp = last_order_gateway_in_timestamp;
    this->last_capture_server_in_timestamp = last_capture_server_in_timestamp;
    this->last_streamer_in_timestamp = last_streamer_in_timestamp;
    this->data=data;
    this->source_id_trigger=source_id_trigger;
}

map<string,vector<double>> OrderBookSnapshot::get_data() const {
  return this->data;
}


Trade::Trade():MarketEvent(),side(),trade_price(),base_quantity() {}
Trade::Trade(const int64_t &last_order_gateway_in_timestamp, const int64_t &last_capture_server_in_timestamp, const int64_t &last_streamer_in_timestamp, const int &source_id_trigger, const int &side, const double &trade_price, const double &base_quantity) {
    this->last_order_gateway_in_timestamp = last_order_gateway_in_timestamp;
    this->last_capture_server_in_timestamp = last_capture_server_in_timestamp;
    this->last_streamer_in_timestamp = last_streamer_in_timestamp;
    this->source_id_trigger=source_id_trigger;
    this->side=side;
    this->trade_price=trade_price;
    this->base_quantity=base_quantity;
}

int Trade::get_side() const {
    return this->side;
}
double Trade::get_trade_price() const {
    return this->trade_price;
}

double Trade::get_base_quantity() const {
    return this->base_quantity;
}



