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

Vwap::Vwap():MarketConsumer(),value(std::nan("")),size(0.0),ask_vwap(std::nan("")),bid_vwap(std::nan("")) {}

Vwap::Vwap(const string& instrument, const string& exchange, double const& size)
    : MarketConsumer(instrument, exchange),
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
    const bool was_valid = this->valid;
    const double last_value = this->value;

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
        return finish();
    }

    double ask_notional = 0.0;
    double bid_notional = 0.0;
    double ask_filled = 0.0;
    double bid_filled = 0.0;

    for (std::size_t i = 0; i < ask_levels && ask_filled < this->size; ++i) {
        const double price = market->ask_price(i);
        const double size = market->ask_size(i);
        if (!std::isfinite(price) || !std::isfinite(size) || size <= 0.0) {
            continue;
        }
        if (ask_filled >= this->size) {
            break;
        }
        const double take = std::min(this->size - ask_filled, size);
        ask_notional += price * take;
        ask_filled += take;
    }

    for (std::size_t i = 0; i < bid_levels && bid_filled < this->size; ++i) {
        const double price = market->bid_price(i);
        const double size = market->bid_size(i);
        if (!std::isfinite(price) || !std::isfinite(size) || size <= 0.0) {
            continue;
        }
        if (bid_filled >= this->size) {
            break;
        }
        const double take = std::min(this->size - bid_filled, size);
        bid_notional += price * take;
        bid_filled += take;
    }

    if (ask_filled < this->size || bid_filled < this->size) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return finish();
    }

    this->ask_vwap = ask_notional / this->size;
    this->bid_vwap = bid_notional / this->size;
    this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
    this->valid = true;
    return finish();
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
