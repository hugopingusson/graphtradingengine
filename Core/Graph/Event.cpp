//
// Created by hugo on 25/03/25.
//

#include "Event.h"
#include "../Node/Base/MarketNode.h"




Event::Event()
    : reception_timestamp(int64_t()),
      source_id_trigger(),
      location(Location::UNKNOWN),
      listener(Listener::UNKNOWN) {}

Event::Event(const int64_t& reception_timestamp,
             const int& source_id_trigger,
             const Location& location,
             const Listener& listener)
    : reception_timestamp(reception_timestamp),
      source_id_trigger(source_id_trigger),
      location(location),
      listener(listener) {}

int64_t Event::get_reception_timestamp() const {
    return this->reception_timestamp;
}

Location Event::get_location() const {
    return this->location;
}

Listener Event::get_listener() const {
    return this->listener;
}

int Event::get_source_id_trigger() const {
    return this->source_id_trigger;
}



HeartBeatEvent::HeartBeatEvent():frequency(){}
HeartBeatEvent::HeartBeatEvent(const int64_t& reception_timestamp,
                               const int& source_id_trigger,
                               const double& clock_frequency,
                               const Location& location,
                               const Listener& listener)
    : Event(reception_timestamp,source_id_trigger,location,listener),
      frequency(clock_frequency) {}


double HeartBeatEvent::get_frequency() const {
    return this->frequency;
}



MarketEvent::MarketEvent():Event(),instrument() {}
MarketEvent::MarketEvent(const string& instrument,
                         const int64_t& reception_timestamp,
                         const int& source_id_trigger,
                         const Location& location,
                         const Listener& listener)
    : Event(reception_timestamp,source_id_trigger,location,listener),
      instrument(instrument) {}

const string& MarketEvent::get_instrument() const {
    return this->instrument;
}

// Double-dispatch implementations
void HeartBeatEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void MarketEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void MarketByPriceEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void SnapshotEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void OrderEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void UpdateEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

void UpdateBatchEvent::dispatchTo(Market& target) {
    target.handle(*this);
}

MarketByPriceEvent::MarketByPriceEvent():MarketEvent(),message() {}
MarketByPriceEvent::MarketByPriceEvent(const string& instrument,
                                       const int64_t& reception_timestamp,
                                       const int& source_id_trigger,
                                       const MarketByPriceMessage& message,
                                       const Location& location,
                                       const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      message(message) {}

MarketByPriceMessage MarketByPriceEvent::get_message() const {
    return this->message;
}

SnapshotData MarketByPriceEvent::get_snapshot_data() const {
    return this->message.order_book_snapshot_data;
}

Order MarketByPriceEvent::get_order() const {
    return this->message.order;
}

MarketTimeStamp MarketByPriceEvent::get_last_market_timestamp() const {
    return this->message.market_time_stamp;
}

SnapshotEvent::SnapshotEvent():MarketEvent(),mbp_message() {}
SnapshotEvent::SnapshotEvent(const string& instrument,
                             const int64_t& reception_timestamp,
                             const int& source_id_trigger,
                             const SnapshotMessage& mbp_message,
                             const Location& location,
                             const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      mbp_message(mbp_message) {}

SnapshotMessage SnapshotEvent::get_message() const {
  return this->mbp_message;
}

SnapshotData SnapshotEvent::get_snapshot_data() const {
    return this->mbp_message.order_book_snapshot_data;
}

MarketTimeStamp SnapshotEvent::get_last_market_timestamp() const {
    return this->mbp_message.market_time_stamp;
}


OrderEvent::OrderEvent():MarketEvent(),mbo_message() {}
OrderEvent::OrderEvent(const string& instrument,
                       const int64_t& reception_timestamp,
                       const int& source_id_trigger,
                       const OrderMessage& mbo_message,
                       const Location& location,
                       const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      mbo_message(mbo_message) {}

OrderMessage OrderEvent::get_message() const {
    return this->mbo_message;
}
Order OrderEvent::get_order() const {
    return this->mbo_message.order;
}

MarketTimeStamp OrderEvent::get_last_market_timestamp() const {
    return this->mbo_message.market_time_stamp;
}

UpdateEvent::UpdateEvent():MarketEvent(),update_message() {}
UpdateEvent::UpdateEvent(const string& instrument,
                         const int64_t& reception_timestamp,
                         const int& source_id_trigger,
                         const UpdateMessage& update_message,
                         const Location& location,
                         const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      update_message(update_message) {}


UpdateMessage UpdateEvent::get_message() const {
    return this->update_message;
}

Update UpdateEvent::get_update() const {
    return this->update_message.update;
}

MarketTimeStamp UpdateEvent::get_last_market_timestamp() const {
    return this->update_message.market_time_stamp;
}

UpdateBatchEvent::UpdateBatchEvent():MarketEvent(),update_messages() {}
UpdateBatchEvent::UpdateBatchEvent(const string& instrument,
                                   const int64_t& reception_timestamp,
                                   const int& source_id_trigger,
                                   const vector<UpdateMessage>& update_messages,
                                   const Location& location,
                                   const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      update_messages(update_messages) {}

const vector<UpdateMessage>& UpdateBatchEvent::get_messages() const {
    return this->update_messages;
}

size_t UpdateBatchEvent::get_batch_size() const {
    return this->update_messages.size();
}

MarketTimeStamp UpdateBatchEvent::get_last_market_timestamp() const {
    if (this->update_messages.empty()) {
        return MarketTimeStamp{};
    }
    return this->update_messages.back().market_time_stamp;
}





TradeEvent::TradeEvent():MarketEvent(),side(),trade_price(),base_quantity(),trade_market_timestamp() {}
TradeEvent::TradeEvent(const string& instrument,
                       const MarketTimeStamp& market_time_stamp,
                       const int64_t& reception_timestamp,
                       const int& source_id_trigger,
                       const Side& side,
                       const double& trade_price,
                       const double& base_quantity,
                       const Location& location,
                       const Listener& listener)
    : MarketEvent(instrument,reception_timestamp,source_id_trigger,location,listener),
      trade_market_timestamp(market_time_stamp){
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

MarketTimeStamp TradeEvent::get_last_market_timestamp() const {
    return this->trade_market_timestamp;
}
