//
// Created by hugo on 25/03/25.
//

#include "Event.h"
#include "../Node/Base/MarketNode.h"




Event::Event():last_streamer_in_timestamp(int64_t()),source_id_trigger(){};
Event::Event(const int64_t &last_streamer_in_timestamp,const int& source_id_trigger):last_streamer_in_timestamp(last_streamer_in_timestamp),source_id_trigger(source_id_trigger) {};

int64_t Event::get_last_streamer_in_timestamp() const {
  return this->last_streamer_in_timestamp;
}

int Event::get_source_id_trigger() const {
    return this->source_id_trigger;
}



HeartBeatEvent::HeartBeatEvent():frequency(){};
HeartBeatEvent::HeartBeatEvent(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency):Event(last_streamer_in_timestamp,source_id_trigger),frequency(clock_frequency){};


double HeartBeatEvent::get_frequency() const {
    return this->frequency;
}



MarketEvent::MarketEvent():Event(),market_timestamp() {};
// MarketEvent::MarketEvent(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger):Event(last_streamer_in_timestamp,source_id_trigger),last_order_gateway_in_timestamp(last_order_gateway_in_timestamp),last_capture_server_in_timestamp(last_capture_server_in_timestamp) {};
MarketEvent::MarketEvent(const MarketTimeStamp& market_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger):Event(last_streamer_in_timestamp,source_id_trigger),market_timestamp(market_timestamp) {};

MarketTimeStamp MarketEvent::get_last_market_timestamp() const {
    return this->market_timestamp;
}

// Double-dispatch implementations
void HeartBeatEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

void MarketEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

void OrderBookSnapshotEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

void IncrementalEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}


OrderBookSnapshotEvent::OrderBookSnapshotEvent():MarketEvent(),mbp_message() {}
OrderBookSnapshotEvent::OrderBookSnapshotEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const MarketByPriceMessage& mbp_message):MarketEvent(market_time_stamp,last_streamer_in_timestamp,source_id_trigger),mbp_message(mbp_message){}

MarketByPriceMessage OrderBookSnapshotEvent::get_message() const {
  return this->mbp_message;
}

SnapshotData OrderBookSnapshotEvent::get_snapshot_data() const {
    return this->mbp_message.order_book_snapshot_data;
}


IncrementalEvent::IncrementalEvent():MarketEvent(),mbo_message() {}
IncrementalEvent::IncrementalEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const MarketByOrderMessage& mbo_message):MarketEvent(market_time_stamp,last_streamer_in_timestamp,source_id_trigger),mbo_message(mbo_message){}

MarketByOrderMessage IncrementalEvent::get_message() const {
    return this->mbo_message;
}
ActionData IncrementalEvent::get_action_data() const {
    return this->mbo_message.action_data;
}

TradeEvent::TradeEvent():MarketEvent(),side(),trade_price(),base_quantity() {}
TradeEvent::TradeEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const Side& side,const double& trade_price,const double& base_quantity):MarketEvent(market_time_stamp,last_streamer_in_timestamp,source_id_trigger){
    this->side=side;
    this->trade_price=trade_price;
    this->base_quantity=base_quantity;
}

Side TradeEvent::get_side() const {
    return this->side;
}
double TradeEvent::get_trade_price() const {
    return this->trade_price;
}

double TradeEvent::get_base_quantity() const {
    return this->base_quantity;
}



