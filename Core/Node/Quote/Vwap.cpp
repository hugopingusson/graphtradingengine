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

    const bool was_valid = this->valid;
    const double last_value = this->value;
    const double last_ask = this->ask_price;
    const double last_bid = this->bid_price;

    auto finish = [&]() -> bool {
        if (this->valid != was_valid) {
            return true;
        }
        if (!this->valid) {
            return false;
        }
        return this->value != last_value;
    };

    auto* market = this->market_parent;

    this->ask_price = market->get_best_ask_price();
    this->bid_price = market->get_best_bid_price();
    const bool quote_changed = (this->ask_price != last_ask) || (this->bid_price != last_bid);

    if (!quote_is_valid(this->ask_price, this->bid_price)) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        if (this->valid != was_valid) {
            return true;
        }
        return quote_changed;
    }

    const std::size_t ask_levels = market->get_ask_level_count();
    const std::size_t bid_levels = market->get_bid_level_count();

    if (this->size <= 0.0) {
        if (ask_levels == 0 || bid_levels == 0) {
            this->ask_vwap = std::nan("");
            this->bid_vwap = std::nan("");
            this->value = std::nan("");
            this->valid = false;
            return finish();
        }
        this->ask_vwap = market->ask_price(0);
        this->bid_vwap = market->bid_price(0);
        this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
        this->valid = true;
        return finish() || quote_changed;
    }

    double ask_notional = 0.0;
    double bid_notional = 0.0;
    double ask_filled = 0.0;
    double bid_filled = 0.0;

    for (std::size_t i = 0; i < ask_levels && ask_filled < this->size; ++i) {
        const double price = market->ask_price(i);
        const double level_size = market->ask_size(i);
        if (!std::isfinite(price) || !std::isfinite(level_size) || level_size <= 0.0) {
            continue;
        }
        if (ask_filled >= this->size) {
            break;
        }
        const double take = std::min(this->size - ask_filled, level_size);
        ask_notional += price * take;
        ask_filled += take;
    }

    for (std::size_t i = 0; i < bid_levels && bid_filled < this->size; ++i) {
        const double price = market->bid_price(i);
        const double level_size = market->bid_size(i);
        if (!std::isfinite(price) || !std::isfinite(level_size) || level_size <= 0.0) {
            continue;
        }
        if (bid_filled >= this->size) {
            break;
        }
        const double take = std::min(this->size - bid_filled, level_size);
        bid_notional += price * take;
        bid_filled += take;
    }

    if (ask_filled < this->size || bid_filled < this->size) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return finish() || quote_changed;
    }

    this->ask_vwap = ask_notional / this->size;
    this->bid_vwap = bid_notional / this->size;
    this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
    this->valid = true;
    return finish() || quote_changed;
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
