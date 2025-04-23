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

MarketOrderBook::MarketOrderBook():depth(),tick_value(),heart_beat_node(nullptr){};
MarketOrderBook::MarketOrderBook(const string &instrument, const string& exchange, const int& depth, const double& tick_value):Node(fmt::format("MarketOrderBook(instrument={},exchange={})",instrument,exchange)),Market(instrument,exchange),depth(depth),tick_value(tick_value),heart_beat_node(nullptr){};
MarketOrderBook::MarketOrderBook(const string &instrument, const string& exchange, const int& depth, const double& tick_value,HeartBeat* heart_beat_node):MarketOrderBook(instrument,exchange,depth,tick_value) {
    this->heart_beat_node=heart_beat_node;
};



OrderBookSnapshotData MarketOrderBook::get_data() {
    return this->data;
}

double MarketOrderBook::get_tick_value() {
    return this->tick_value;
}

int MarketOrderBook::get_depth() {
    return this->depth;
}


double MarketOrderBook::ask_size(const int& i) const {
    return data.ask_size[i];
}

double MarketOrderBook::ask_price(const int& i) const {
    return data.ask_price[i];
}

double MarketOrderBook::bid_size(const int& i) const {
    return data.bid_size[i];
}

double MarketOrderBook::bid_price(const int& i) const {
    return data.bid_price[i];
}


double MarketOrderBook::mid() const {
    return 0.5*(this->ask_price(0)+this->bid_price(0));
}

double MarketOrderBook::spread() const {
    return this->ask_price(0)-this->bid_price(0);
}

double MarketOrderBook::imbalance() const {
    return (this->ask_size(0)-this->bid_size(0))/(this->ask_size(0)+this->bid_size(0));
}

double MarketOrderBook::bary() const {
    return this->mid()+0.5*this->spread()*this->imbalance();
}

double MarketOrderBook::bid_amount(const int& i) const {
    return this->bid_size(i)*this->bid_price(i);
}
double MarketOrderBook::ask_amount(const int& i) const {
    return this->ask_size(i)*this->ask_price(i);
}

double MarketOrderBook::bid_amount_at_mid(const int& i) const {
    return this->bid_size(i)*this->mid();
}
double MarketOrderBook::ask_amount_at_mid(const int& i) const {
    return this->ask_size(i)*this->mid();
}

double MarketOrderBook::cumulative_ask_size(const int& i) const {
    double size=0;
    for (int k=0;k<=i;k++) {
        size+=this->ask_size(i);
    }

    return size;
}

double MarketOrderBook::cumulative_bid_size(const int& i) const {
    double size=0;
    for (int k=0;k<=i;k++) {
        size+=this->bid_size(i);
    }

    return size;
}

double MarketOrderBook::cumulative_ask_amount(const int& i) const {
    double amount=0;
    for (int k=0;k<=i;k++) {
        amount+=this->ask_amount(i);
    }

    return amount;
}

double MarketOrderBook::cumulative_bid_amount(const int& i) const {
    double amount=0;
    for (int k=0;k<=i;k++) {
        amount+=this->bid_amount(i);
    }

    return amount;
}


// void MarketOrderBook::handle(OrderBookSnapshotEvent& order_book_snapshot) {
void MarketOrderBook::on_event(Event* event) {
    OrderBookSnapshotEvent* order_book_snapshot = dynamic_cast<OrderBookSnapshotEvent*>(event);
	this->last_streamer_in_timestamp = order_book_snapshot->get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = order_book_snapshot->get_last_market_timestamp().capture_server_in_timestamp;
    this->last_order_gateway_in_timestamp = order_book_snapshot->get_last_market_timestamp().order_gateway_in_timestamp;
    this->data = order_book_snapshot->get_order_book_snapshot_data();

    if (!this->check_snapshot()) {
      this->valid = false;
    }
    else{
      this->valid = true;
     }

}

void MarketOrderBook::update() {
    if (this->heart_beat_node->get_last_streamer_in_timestamp()-this->last_streamer_in_timestamp > 3*1e6) {
        this->logger->log_info("MarketOrderBook", fmt::format("{} is now stale, setting to invalid, last streamer in was 3 sec ago at ", this->name,this->time_helper.convert_nanoseconds_to_string(this->last_streamer_in_timestamp)));
        this->valid = false;

    }
}



bool MarketOrderBook::check_snapshot() {

  if (this->ask_price(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("ask price received by {} is 0, setting to invalid",this->name));
    return false;
  }

  if (this->bid_price(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("bid price received by {} is 0, setting to invalid",this->name));
    return false;
  }

  if (this->ask_size(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("ask size received by {} is 0, setting to invalid",this->name));
    return false;
  }

  if (this->bid_size(0)==0){
    this->logger->log_error("MarketOrderBook",fmt::format("bid size received by {} is 0, setting to invalid",this->name));
    return false;
  }

  if (this->ask_price(0)>=this->bid_price(0)){
    this->logger->log_error("MarketOrderBook",fmt::format("snapshot received by {} show ask={}>=bid={} , setting to invalid",this->name,this->ask_price(0),this->bid_price(0)));
  	return false;
  }

  return true;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////

MarketTrade::MarketTrade():trade_price(),side(),base_quantity(){}
MarketTrade::MarketTrade(const string &instrument, const string &exchange):Node(fmt::format("MarketTrade(instrument={},exchange={})",instrument,exchange)),Market(instrument,exchange),trade_price(),side(),base_quantity(){}


int MarketTrade::get_side() {
    return this->side;
}

double MarketTrade::get_base_quantity() {
    return this->base_quantity;
}

double MarketTrade::get_trade_price() {
    return this->trade_price;
}

double MarketTrade::get_quote_quantity() {
    return this->base_quantity*this->trade_price;
}


// void MarketTrade::handle(Trade& trade) {
void MarketTrade::on_event(Event* event) {
    Trade* trade = dynamic_cast<Trade*>(event);
    this->last_streamer_in_timestamp = trade->get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = trade->get_last_market_timestamp().capture_server_in_timestamp;
    this->last_order_gateway_in_timestamp = trade->get_last_market_timestamp().order_gateway_in_timestamp;

    this->side = trade->get_side();
    this->trade_price = trade->get_trade_price();
    this->base_quantity = trade->get_base_quantity();

    if (!this->check_trade()) {
        this->valid = false;
    }
    else{
        this->valid = true;
    }

}

bool MarketTrade::check_trade() {

    if (this->trade_price<0){
        this->logger->log_error("MarketTrade",fmt::format("trade price received by {} is less than 0, setting to invalid",this->name));
        return false;
    }

    if (std::abs(this->side)!=1){
        this->logger->log_error("MarketTrade",fmt::format("side received by {} is different from 1 or -1, setting to invalid",this->name));
        return false;
    }

    if (this->base_quantity<0){
        this->logger->log_error("MarketTrade",fmt::format("base quantity received by {} less than 0, setting to invalid",this->name));
        return false;
    }


    return true;

};