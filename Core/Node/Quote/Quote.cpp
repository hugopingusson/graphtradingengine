//
// Created by hugo on 29/04/26.
//

#include "Quote.h"

Quote::Quote() : ask_price(), bid_price() {}

double Quote::get_ask_price() {
    return this->ask_price;
}

double Quote::get_bid_price() {
    return this->bid_price;
}

double Quote::mid() {
    return 0.5 * (this->ask_price + this->bid_price);
}

double Quote::spread() {
    return this->ask_price - this->bid_price;
}

MarketQuote::MarketQuote()
    : Quote(),
      MarketConsumer() {}

MarketQuote::MarketQuote(const string& instrument,
                         const string& exchange)
    : Quote(),
      MarketConsumer(instrument, exchange) {}
