//
// Created by hugo on 25/03/25.
//

#include "MarketNode.h"

Market::Market():SourceNode(){};
Market::Market(const string& name,const string& instrument, const string& exchange):SourceNode(name),instrument(instrument),exchange(exchange) {}


string Market::get_instrument() {
    return this->instrument;
}

string Market::get_exchange() {
    return this->exchange;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

MarketOrderBook::MarketOrderBook():Market(),depth(int()),tick_value(double()){};
MarketOrderBook::MarketOrderBook(const string &instrument, const string& exchange, const int& depth, const double& tick_value):Market(fmt::format("MarketOrderBook(instrument={},exchange={})",instrument,exchange),instrument,exchange),depth(depth),tick_value(tick_value){};



map<string,vector<double>> MarketOrderBook::get_data() {
    return this->data;
}

double MarketOrderBook::get_tick_value() {
    return this->tick_value;
}

int MarketOrderBook::get_depth() {
    return this->depth;
}


double MarketOrderBook::ask_size(const int& i) const {
    return data.at("ask_size").at(i);
}

double MarketOrderBook::ask_price(const int& i) const {
    return data.at("ask_price").at(i);
}

double MarketOrderBook::bid_size(const int& i) const {
    return data.at("bid_size").at(i);
}

double MarketOrderBook::bid_price(const int& i) const {
    return data.at("bid_price").at(i);
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


void MarketOrderBook::update(OrderBookSnapshot* order_book_snapshot) {
	this->last_streamer_in_timestamp = order_book_snapshot->get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = order_book_snapshot->get_last_capture_server_in_timestamp();
    this->last_order_gateway_in_timestamp = order_book_snapshot->get_last_order_gateway_in_timestamp();

    this->data = order_book_snapshot->get_data();

    if (!this->check_snapshot()) {
      this->valid = false;
    }
    else{
      this->valid = true;
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

MarketTrade::MarketTrade():Market(),trade_price(),side(),base_quantity(){}
MarketTrade::MarketTrade(const string &instrument, const string &exchange):Market(fmt::format("MarketTrade(instrument={},exchange={})",instrument,exchange),instrument,exchange),trade_price(),side(),base_quantity(){}


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


void MarketTrade::update(Trade* trade) {
    this->last_streamer_in_timestamp = trade->get_last_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = trade->get_last_capture_server_in_timestamp();
    this->last_order_gateway_in_timestamp = trade->get_last_order_gateway_in_timestamp();

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