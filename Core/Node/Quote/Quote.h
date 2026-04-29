//
// Created by hugo on 29/04/26.
//

#ifndef QUOTE_H
#define QUOTE_H

#include "../Base/Node.h"

class Quote : public virtual Consumer {
public:
    ~Quote() override = default;
    Quote();

    double get_ask_price();
    double get_bid_price();

    double mid();
    double spread();

protected:
    double ask_price;
    double bid_price;
};

class MarketQuote : public Quote, public MarketConsumer {
public:
    ~MarketQuote() override = default;
    MarketQuote();
    explicit MarketQuote(const string& instrument,
                         const string& exchange);
};

#endif //QUOTE_H
