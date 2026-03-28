//
// Created by hugo on 25/03/25.
//

#include "MarketNode.h"

#include <iostream>

Market::Market(){};
Market::Market(const string& instrument, const string& exchange):instrument(instrument),exchange(exchange) {}


string Market::get_instrument() {
    return this->instrument;
}

string Market::get_exchange() {
    return this->exchange;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

MarketOrderBook::MarketOrderBook():depth(),tick_value(){};
MarketOrderBook::MarketOrderBook(const string &instrument, const string& exchange, const int& depth, const double& tick_value):Node(fmt::format("MarketOrderBook(instrument={},exchange={})",instrument,exchange)),Market(instrument,exchange),depth(depth),tick_value(tick_value){};


AskLadder MarketOrderBook::get_ask_ladder() {
    return this->ask_ladder;
}

BidLadder MarketOrderBook::get_bid_ladder() {
    return this->bid_ladder;
}

double MarketOrderBook::get_tick_value() {
    return this->tick_value;
}

int MarketOrderBook::get_depth() {
    return this->depth;
}


double MarketOrderBook::ask_price(const std::size_t& i) const {
    std::size_t cpt = 0;
    double price;
    for (auto layer=this->ask_ladder.begin();layer!=this->ask_ladder.end() && cpt<=i;layer++,cpt++) {
        price=layer->first;
        cpt++;
    }
    return price;
}

double MarketOrderBook::bid_price(const std::size_t& i) const {
    std::size_t cpt = 0;
    double price;
    for (auto layer=this->bid_ladder.begin();layer!=this->bid_ladder.end() && cpt<=i;layer++,cpt++) {
        price=layer->first;
        cpt++;
    }
    return price;
}

double MarketOrderBook::ask_size(const std::size_t& i) const {
    std::size_t cpt = 0;
    double size;
    for (auto layer=this->ask_ladder.begin();layer!=this->ask_ladder.end() && cpt<=i;layer++,cpt++) {
        size=layer->second.size;
        cpt++;
    }
    return size;
}

double MarketOrderBook::bid_size(const std::size_t& i) const {
    std::size_t cpt = 0;
    double size;
    for (auto layer=this->bid_ladder.begin();layer!=this->bid_ladder.end() && cpt<=i;layer++,cpt++) {
        size=layer->second.size;
        cpt++;
    }
    return size;
}


double MarketOrderBook::get_best_ask_price() const {
    return ask_ladder.begin()->first;
}
double MarketOrderBook::get_best_bid_price() const {
    return bid_ladder.begin()->first;
}

double MarketOrderBook::get_best_ask_size() const {
    return ask_ladder.begin()->second.size;
}
double MarketOrderBook::get_best_bid_size() const {
    return bid_ladder.begin()->second.size;
}

double MarketOrderBook::mid() const {
    return 0.5*(this->get_best_ask_price()+this->get_best_bid_price());
}

double MarketOrderBook::spread() const {
    return this->get_best_ask_price()-this->get_best_bid_price();
}

double MarketOrderBook::imbalance() const {
    return (this->get_best_ask_size()-this->get_best_bid_price())/(this->get_best_ask_size()+this->get_best_bid_price());
}

double MarketOrderBook::bary() const {
    return this->mid()+0.5*this->spread()*this->imbalance();
}

// Double-dispatch entry
void MarketOrderBook::on_event(Event* event) {
    if (!event) return;
    event->dispatchTo(*this);
}

// Handlers
void MarketOrderBook::handle(HeartBeatEvent& hb) {
    if (hb.get_last_streamer_in_timestamp()-this->last_streamer_in_timestamp > 3*1e6) {
        this->logger->log_info("MarketOrderBook", fmt::format("{} is now stale, setting to invalid, last streamer in was 3 sec ago at ", this->name,this->time_helper.convert_nanoseconds_to_string(this->last_streamer_in_timestamp)));
        this->valid=false;
    }
}

void MarketOrderBook::handle(MarketEvent& ev) {
    // Par défaut, ne rien faire. Les sous-types spécifiques traiteront.
}

// Specific handler for OrderBookSnapshotEvent
void MarketOrderBook::handle(OrderBookSnapshotEvent& order_book_snapshot) {
    this->last_streamer_in_timestamp = order_book_snapshot.get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = order_book_snapshot.get_last_market_timestamp().capture_server_in_timestamp;
    this->last_order_gateway_in_timestamp = order_book_snapshot.get_last_market_timestamp().order_gateway_in_timestamp;
    snapshot_to_ladder(order_book_snapshot.get_snapshot_data(),this->bid_ladder,this->ask_ladder);
    this->valid=check_snapshot();
}

void MarketOrderBook::handle(IncrementalEvent& incremental_event) {
    this->last_streamer_in_timestamp = incremental_event.get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = incremental_event.get_last_market_timestamp().capture_server_in_timestamp;
    this->last_order_gateway_in_timestamp = incremental_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->match()
    this->valid=check_snapshot();
}

bool MarketOrderBook::check_snapshot() {
  if (this->ask_price(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("ask price received @ {} by {} is 0, setting to invalid",time_helper.convert_nanoseconds_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    return false;
  }

  if (this->bid_price(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("bid price received @ {} by {} is 0, setting to invalid",time_helper.convert_nanoseconds_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    return false;
  }

  if (this->ask_size(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("ask size received @ {} by {} is 0, setting to invalid",time_helper.convert_nanoseconds_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    return false;
  }

  if (this->bid_size(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("bid size received @ {} by {} is 0, setting to invalid",time_helper.convert_nanoseconds_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    return false;
  }

  if (this->bid_price(0)>=this->ask_price(0)){
    this->logger->log_error("MarketOrderBook",fmt::format("snapshot received @ {} by {} show bid={}>=ask={} , setting to invalid",time_helper.convert_nanoseconds_to_string(this->get_last_order_gateway_in_timestamp()),this->name,this->bid_price(0),this->ask_price(0)));
  	return false;
  }

  return true;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////

// MarketTrade::MarketTrade():trade_price(),side(),base_quantity(){}
// MarketTrade::MarketTrade(const string &instrument, const string &exchange):Node(fmt::format("MarketTrade(instrument={},exchange={})",instrument,exchange)),Market(instrument,exchange),trade_price(),side(),base_quantity(){}
//
//
// Side MarketTrade::get_side() {
//     return this->side;
// }
//
// double MarketTrade::get_base_quantity() {
//     return this->base_quantity;
// }
//
// double MarketTrade::get_trade_price() {
//     return this->trade_price;
// }
//
// double MarketTrade::get_quote_quantity() {
//     return this->base_quantity*this->trade_price;
// }
//
//
// // void MarketTrade::handle(TradeEvent& trade) {
// void MarketTrade::on_event(Event* event) {
//     TradeEvent* trade = dynamic_cast<TradeEvent*>(event);
//     this->last_streamer_in_timestamp = trade->get_last_streamer_in_timestamp();
//     this->last_capture_server_in_timestamp = trade->get_last_market_timestamp().capture_server_in_timestamp;
//     this->last_order_gateway_in_timestamp = trade->get_last_market_timestamp().order_gateway_in_timestamp;
//
//     this->side = trade->get_side();
//     this->trade_price = trade->get_trade_price();
//     this->base_quantity = trade->get_base_quantity();
//
//     if (!this->check_trade()) {
//         this->valid = false;
//     }
//     else{
//         this->valid = true;
//     }
//
// }
//
// bool MarketTrade::check_trade() {
//
//     if (this->trade_price<0){
//         this->logger->log_error("MarketTrade",fmt::format("trade price received by {} is less than 0, setting to invalid",this->name));
//         return false;
//     }
//
//     if (this->side != Side::ASK & this->side != Side::BID){
//         this->logger->log_error("MarketTrade",fmt::format("side received by {} is different from 1 or -1, setting to invalid",this->name));
//         return false;
//     }
//
//     if (this->base_quantity<0){
//         this->logger->log_error("MarketTrade",fmt::format("base quantity received by {} less than 0, setting to invalid",this->name));
//         return false;
//     }
//
//
//     return true;
//
// };