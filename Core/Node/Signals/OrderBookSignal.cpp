//
// Created by hugo on 01/04/25.
//

#include "OrderBookSignal.h"

#include <algorithm>
#include <cmath>
#include <fmt/format.h>


Mid::Mid():SingleInputConsumer(),market(nullptr) {}

Mid::Mid(MarketOrderBook* market):SingleInputConsumer(market),market(market) {
    this->name=fmt::format("Mid({}@{})",market->get_instrument(),market->get_exchange());
    this->mark_dirty();
}

void Mid::compute() {
    if (!this->market) {
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    this->value = this->market->mid();
    this->valid = !std::isnan(this->value);
}

Bary::Bary():SingleInputConsumer(),market(nullptr) {}

Bary::Bary(MarketOrderBook* market):SingleInputConsumer(market),market(market) {
    this->name=fmt::format("Bary({}@{})",market->get_instrument(),market->get_exchange());
    this->mark_dirty();
}

void Bary::compute() {
    if (!this->market) {
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    this->value = this->market->bary();
    this->valid = !std::isnan(this->value);
}

Vwap::Vwap():SingleInputConsumer(),market(nullptr),amount(0.0),ask_vwap(std::nan("")),bid_vwap(std::nan("")) {}

Vwap::Vwap(MarketOrderBook *market,double const& amount):SingleInputConsumer(market),market(market),amount(amount),ask_vwap(std::nan("")),bid_vwap(std::nan("")) {
    this->name=fmt::format("Vwap({}@{};amount={})",market->get_instrument(),market->get_exchange(),amount);
    this->mark_dirty();
}

void Vwap::compute() {
    if (!this->market) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    const auto& asks = this->market->get_ask_ladder();
    const auto& bids = this->market->get_bid_ladder();

    if (this->amount <= 0.0) {
        if (asks.empty() || bids.empty()) {
            this->ask_vwap = std::nan("");
            this->bid_vwap = std::nan("");
            this->value = std::nan("");
            this->valid = false;
            return;
        }
        this->ask_vwap = asks.begin()->first;
        this->bid_vwap = bids.begin()->first;
        this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
        this->valid = true;
        return;
    }

    double ask_notional = 0.0;
    double bid_notional = 0.0;
    double ask_filled = 0.0;
    double bid_filled = 0.0;

    for (const auto& [price, level] : asks) {
        if (ask_filled >= this->amount) {
            break;
        }
        if (level.amount <= 0.0) {
            continue;
        }
        const double take = std::min(this->amount - ask_filled, level.amount);
        ask_notional += price * take;
        ask_filled += take;
    }

    for (const auto& [price, level] : bids) {
        if (bid_filled >= this->amount) {
            break;
        }
        if (level.amount <= 0.0) {
            continue;
        }
        const double take = std::min(this->amount - bid_filled, level.amount);
        bid_notional += price * take;
        bid_filled += take;
    }

    if (ask_filled < this->amount || bid_filled < this->amount) {
        this->ask_vwap = std::nan("");
        this->bid_vwap = std::nan("");
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    this->ask_vwap = ask_notional / this->amount;
    this->bid_vwap = bid_notional / this->amount;
    this->value = 0.5 * (this->ask_vwap + this->bid_vwap);
    this->valid = true;
}

double Vwap::get_ask_vwap() const {
    return this->ask_vwap;
}

double Vwap::get_bid_vwap() const {
    return this->bid_vwap;
}

TopOfBookImbalance::TopOfBookImbalance() : SingleInputConsumer(), market(nullptr) {}

TopOfBookImbalance::TopOfBookImbalance(MarketOrderBook* market)
    : SingleInputConsumer(market), market(market) {
    this->name = fmt::format("TopOfBookImbalance({}@{})", market->get_instrument(), market->get_exchange());
    this->mark_dirty();
}

void TopOfBookImbalance::compute() {
    if (!this->market) {
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    const double best_bid_size = this->market->get_best_bid_size();
    const double best_ask_size = this->market->get_best_ask_size();
    const double denom = best_bid_size + best_ask_size;

    if (!std::isfinite(best_bid_size) || !std::isfinite(best_ask_size) || denom <= 0.0) {
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    this->value = (best_bid_size - best_ask_size) / denom;
    this->valid = std::isfinite(this->value);
}
