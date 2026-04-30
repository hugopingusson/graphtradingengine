//
// Created by hugo on 29/04/26.
//

#include "Vwap.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <fmt/format.h>

#include "../Base/MarketNode.h"

namespace {
bool quote_is_valid(const double& ask, const double& bid) {
    return std::isfinite(ask)
        && std::isfinite(bid)
        && ask > 0.0
        && bid > 0.0
        && bid < ask;
}
}

Vwap::Vwap()
    : MarketQuote(),
      value(std::nan("")),
      size(0.0),
      ask_vwap(std::nan("")),
      bid_vwap(std::nan("")) {}

Vwap::Vwap(const string& instrument, const string& exchange, double const& size)
    : MarketQuote(instrument, exchange),
      value(std::nan("")),
      size(size),
      ask_vwap(std::nan("")),
      bid_vwap(std::nan("")) {
    this->name = exchange.empty()
        ? fmt::format("Vwap({};size={})", instrument, size)
        : fmt::format("Vwap({}@{};size={})", instrument, exchange, size);
    this->mark_scheduled();
}

bool Vwap::recompute() {
    if (!this->market_parent) {
        throw std::runtime_error("Vwap::recompute called without connected market parent");
    }
    if (this->size <= 0.0) {
        throw std::invalid_argument("Vwap::recompute requires size > 0");
    }

    const bool previous_validity = this->valid;
    const double previous_value = this->value;

    auto* market = this->market_parent;

    this->ask_price = market->get_best_ask_price();
    this->bid_price = market->get_best_bid_price();

    if (!quote_is_valid(this->ask_price, this->bid_price)) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return previous_validity != this->valid;
    }

    const std::size_t depth = market->get_effective_depth();
    if (depth == 0) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return previous_validity != this->valid;
    }

    double ask_notional = 0.0;
    double bid_notional = 0.0;
    double ask_filled = 0.0;
    double bid_filled = 0.0;

    for (std::size_t i = 0; i < depth && ask_filled < this->size; ++i) {
        const double price = market->ask_price(i);
        const double level_size = market->ask_size(i);
        const double take = std::min(this->size - ask_filled, level_size);
        ask_notional += price * take;
        ask_filled += take;
    }

    for (std::size_t i = 0; i < depth && bid_filled < this->size; ++i) {
        const double price = market->bid_price(i);
        const double level_size = market->bid_size(i);
        const double take = std::min(this->size - bid_filled, level_size);
        bid_notional += price * take;
        bid_filled += take;
    }

    if (ask_filled < this->size || bid_filled < this->size) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return previous_validity != this->valid;
    }

    this->ask_vwap = ask_notional / this->size;
    this->bid_vwap = bid_notional / this->size;
    this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
    this->valid = true;
    return (previous_validity != this->valid) || (this->value != previous_value);
}

double Vwap::get_value() const {
    return this->value;
}

double Vwap::get_ask_vwap() const {
    return this->ask_vwap;
}

double Vwap::get_bid_vwap() const {
    return this->bid_vwap;
}
