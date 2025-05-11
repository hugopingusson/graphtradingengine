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