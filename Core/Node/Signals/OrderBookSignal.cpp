//
// Created by hugo on 01/04/25.
//

#include "OrderBookSignal.h"

#include <fmt/format.h>


Mid::Mid(){}

Mid::Mid(MarketOrderBook* market) {
    this->parent=market;
    this->name=fmt::format("Mid({}@{})",market->get_instrument(),market->get_exchange());
}

double Mid::compute() {
    return dynamic_cast<MarketOrderBook*>(this->parent)->mid();
}

Bary::Bary()= default;

Bary::Bary(MarketOrderBook* market) {
    this->parent=market;
    this->name=fmt::format("Bary({}@{})",market->get_instrument(),market->get_exchange());
}

double Bary::compute() {
    return dynamic_cast<MarketOrderBook*>(this->parent)->bary();
}



Vwap::Vwap()=default;

Vwap::Vwap(MarketOrderBook *market,double const& size) {
    this->market=market;
    this->size=size;
    this->name=fmt::format("Vwap({}@{};size={})",market->get_instrument(),market->get_exchange(),size);
}

void Vwap::update() {
    double ask=0;
    double bid=0;

    double ask_size=0;
    double bid_size=0;

    int i_ask=0;
    while (i_ask<=this->market->get_depth()-1 && ask_size<this->size) {
        ask+=this->market->ask_price(i_ask)*min(this->size-ask_size,this->market->ask_size(i_ask));
        ask_size+=min(this->size-ask_size,this->market->ask_size(i_ask));
        i_ask++;
    }

    // if (ask_size < this->size) {
    //     this->logger->log_info("Vwap",fmt::format("maximum depth reached on ask, total volume available = {}",market->cumulative_ask_size(market->get_depth()-1)));
    // }


    int i_bid=0;
    while (i_bid<=this->market->get_depth()-1 && bid_size<this->size) {
        bid+=this->market->bid_price(i_bid)*min(this->size-bid_size,this->market->bid_size(i_bid));
        bid_size+=min(this->size-bid_size,this->market->bid_size(i_bid));
        i_bid++;
    }

    // if (bid_size < this->size) {
    //     this->logger->log_info("Vwap",fmt::format("maximum depth reached on bid, total volume available = {}",market->cumulative_bid_size(market->get_depth()-1)));
    // }


    this->ask_price=ask/ask_size;
    this->bid_price=bid/bid_size;

}


