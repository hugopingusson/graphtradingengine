//
// Created by hugo on 01/04/25.
//

#include "OrderBookSignal.h"

#include <algorithm>
#include <cmath>
#include <fmt/format.h>


Mid::Mid():MarketConsumer(), value(std::nan("")) {}

Mid::Mid(const string& instrument, const string& exchange)
    : MarketConsumer(instrument, exchange), value(std::nan("")) {
    this->name = exchange.empty()
        ? fmt::format("Mid({})", instrument)
        : fmt::format("Mid({}@{})", instrument, exchange);
    this->mark_scheduled();
}

bool Mid::recompute() {
    const bool was_valid = this->valid;
    const double last_value = this->value;

    auto* market = this->market_parent;
    this->value = market->mid();
    this->valid = !std::isnan(this->value);

    if (this->valid != was_valid) {
        return true;
    }
    if (!this->valid) {
        return false;
    }
    return this->value != last_value;
}

double Mid::get_value() const {
    return this->value;
}

Bary::Bary():MarketConsumer(), value(std::nan("")) {}

Bary::Bary(const string& instrument, const string& exchange)
    : MarketConsumer(instrument, exchange), value(std::nan("")) {
    this->name = exchange.empty()
        ? fmt::format("Bary({})", instrument)
        : fmt::format("Bary({}@{})", instrument, exchange);
    this->mark_scheduled();
}

bool Bary::recompute() {
    const bool was_valid = this->valid;
    const double last_value = this->value;

    auto* market = this->market_parent;
    this->value = market->bary();
    this->valid = !std::isnan(this->value);

    if (this->valid != was_valid) {
        return true;
    }
    if (!this->valid) {
        return false;
    }
    return this->value != last_value;
}

double Bary::get_value() const {
    return this->value;
}

TopOfBookImbalance::TopOfBookImbalance() : MarketConsumer(), value(std::nan("")) {}

TopOfBookImbalance::TopOfBookImbalance(const string& instrument, const string& exchange)
    : MarketConsumer(instrument, exchange), value(std::nan("")) {
    this->name = exchange.empty()
        ? fmt::format("TopOfBookImbalance({})", instrument)
        : fmt::format("TopOfBookImbalance({}@{})", instrument, exchange);
    this->mark_scheduled();
}

bool TopOfBookImbalance::recompute() {
    const bool was_valid = this->valid;
    const double last_value = this->value;

    auto* market = this->market_parent;
    const double best_bid_size = market->get_best_bid_size();
    const double best_ask_size = market->get_best_ask_size();
    const double denom = best_bid_size + best_ask_size;

    if (!std::isfinite(best_bid_size) || !std::isfinite(best_ask_size) || denom <= 0.0) {
        this->value = std::nan("");
        this->valid = false;
    } else {
        this->value = (best_bid_size - best_ask_size) / denom;
        this->valid = std::isfinite(this->value);
    }

    if (this->valid != was_valid) {
        return true;
    }
    if (!this->valid) {
        return false;
    }
    return this->value != last_value;
}

double TopOfBookImbalance::get_value() const {
    return this->value;
}
