//
// Created by hugo on 25/03/25.
//

#include "Event.h"
#include "../Node/Base/MarketNode.h"




Event::Event():capture_server_in_timestamp(int64_t()),streamer_in_timestamp(int64_t()),source_id_trigger(){};
Event::Event(const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger):capture_server_in_timestamp(capture_server_in_timestamp),streamer_in_timestamp(streamer_in_timestamp),source_id_trigger(source_id_trigger) {};

int64_t Event::get_capture_server_in_timestamp() const {
    return this->capture_server_in_timestamp;
}

int64_t Event::get_streamer_in_timestamp() const {
  return this->streamer_in_timestamp;
}

int Event::get_source_id_trigger() const {
    return this->source_id_trigger;
}



HeartBeatEvent::HeartBeatEvent():frequency(){};
HeartBeatEvent::HeartBeatEvent(const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency):Event(capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger),frequency(clock_frequency){};


double HeartBeatEvent::get_frequency() const {
    return this->frequency;
}



MarketEvent::MarketEvent():Event(),market_timestamp() {};
MarketEvent::MarketEvent(const string& instrument,const MarketTimeStamp& market_timestamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger):Event(capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger),market_timestamp(market_timestamp),instrument(instrument) {};

const string& MarketEvent::get_instrument() const {
    return this->instrument;
}

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

void MBPEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

void MBOEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

void UpdateEvent::dispatchTo(MarketOrderBook& target) {
    target.handle(*this);
}

MBPEvent::MBPEvent():MarketEvent(),mbp_message() {}
MBPEvent::MBPEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketByPriceMessage& mbp_message):MarketEvent(instrument,market_time_stamp,capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger),mbp_message(mbp_message){}

MarketByPriceMessage MBPEvent::get_message() const {
  return this->mbp_message;
}

SnapshotData MBPEvent::get_snapshot_data() const {
    return this->mbp_message.order_book_snapshot_data;
}


MBOEvent::MBOEvent():MarketEvent(),mbo_message() {}
MBOEvent::MBOEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketByOrderMessage& mbo_message):MarketEvent(instrument,market_time_stamp,capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger),mbo_message(mbo_message){}

MarketByOrderMessage MBOEvent::get_message() const {
    return this->mbo_message;
}
Order MBOEvent::get_order() const {
    return this->mbo_message.order;
}

UpdateEvent::UpdateEvent():MarketEvent(),update_message() {}
UpdateEvent::UpdateEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketUpdateMessage& update_message):MarketEvent(instrument,market_time_stamp,capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger),update_message(update_message){}


MarketUpdateMessage UpdateEvent::get_message() const {
    return this->update_message;
}

Update UpdateEvent::get_update() const {
    return this->update_message.update;
}





TradeEvent::TradeEvent():MarketEvent(),side(),trade_price(),base_quantity() {}
TradeEvent::TradeEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const Side& side,const double& trade_price,const double& base_quantity):MarketEvent(instrument,market_time_stamp,capture_server_in_timestamp,streamer_in_timestamp,source_id_trigger){
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
