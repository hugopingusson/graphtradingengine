//
// Created by hugo on 25/03/25.
//

#include "MarketNode.h"

#include <exception>
#include <limits>

#include "../../../Helper/SaphirManager.h"

Market::Market():instrument(),exchange(),depth(),tick_value() {}
Market::Market(const string& instrument, const string& exchange, const int& depth, const double& tick_value)
    : Node(fmt::format("Market(instrument={},exchange={})", instrument, exchange)),
      instrument(instrument),
      exchange(exchange),
      depth(depth),
      tick_value(tick_value) {
    if (this->depth <= 0) {
        try {
            const SaphirManager saphir;
            this->depth = static_cast<int>(saphir.get_market_depth());
        } catch (const std::exception&) {
            this->depth = static_cast<int>(kBookLevels);
        }
    }
    if (this->depth <= 0) {
        this->depth = 1;
    }
}

string Market::get_instrument() {
    return this->instrument;
}

string Market::get_exchange() {
    return this->exchange;
}

const AskLadder& Market::get_ask_ladder() const {
    return this->ask_ladder;
}

const BidLadder& Market::get_bid_ladder() const {
    return this->bid_ladder;
}

double Market::get_tick_value() {
    return this->tick_value;
}

int Market::get_depth() {
    return this->depth;
}

double Market::ask_price(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->ask_ladder.begin(); it != this->ask_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->first;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double Market::bid_price(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->bid_ladder.begin(); it != this->bid_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->first;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double Market::ask_size(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->ask_ladder.begin(); it != this->ask_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->second.size;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double Market::bid_size(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->bid_ladder.begin(); it != this->bid_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->second.size;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double Market::get_best_ask_price() const {
    if (ask_ladder.empty()) {
        return 0.0;
    }
    return ask_ladder.begin()->first;
}

double Market::get_best_bid_price() const {
    if (bid_ladder.empty()) {
        return 0.0;
    }
    return bid_ladder.begin()->first;
}

double Market::get_best_ask_size() const {
    if (ask_ladder.empty()) {
        return 0.0;
    }
    return ask_ladder.begin()->second.size;
}

double Market::get_best_bid_size() const {
    if (bid_ladder.empty()) {
        return 0.0;
    }
    return bid_ladder.begin()->second.size;
}

double Market::mid() const {
    return 0.5 * (this->get_best_ask_price() + this->get_best_bid_price());
}

double Market::spread() const {
    return this->get_best_ask_price() - this->get_best_bid_price();
}

double Market::imbalance() const {
    const double ask_size = this->get_best_ask_size();
    const double bid_size = this->get_best_bid_size();
    const double denom = ask_size + bid_size;
    if (denom == 0.0) {
        return 0.0;
    }
    return (ask_size - bid_size) / denom;
}

double Market::bary() const {
    return this->mid() + 0.5 * this->spread() * this->imbalance();
}

bool Market::check_staleness(HeartBeatEvent& hb) {
    static constexpr int64_t kStaleThresholdNs = 3'000'000'000LL;
    if (hb.get_reception_timestamp() - this->last_reception_timestamp > kStaleThresholdNs) {
        if (this->logger) {
            this->logger->log_info("Market", fmt::format("{} is now stale, setting to invalid, last reception was 3 sec ago at {}", this->name, Timestamp::unix_to_string(this->last_reception_timestamp)));
        }
        this->valid = false;
        return false;
    }
    return true;
}

void Market::update(BookLevel level, Side side, Action action) {
    if (side == Side::BID) {
        if (action == Action::CANCEL) {
            bid_ladder.erase(level.price);
        }
        if (action == Action::ADD || action == Action::MODIFY) {
            bid_ladder[level.price] = level;
        }
    }
    if (side == Side::ASK) {
        if (action == Action::CANCEL) {
            ask_ladder.erase(level.price);
        }
        if (action == Action::ADD || action == Action::MODIFY) {
            ask_ladder[level.price] = level;
        }
    }
    this->trim_to_depth();
}

bool Market::match(Order order) {
    if (order.action == Action::ADD) {
        if (order.side == Side::ASK) {
            if (ask_ladder.contains(order.price)) {
                ask_ladder[order.price].size += order.size;
                this->trim_to_depth();
                return true;
            }
            ask_ladder[order.price].size = order.size;
            this->trim_to_depth();
            return true;
        }
        if (order.side == Side::BID) {
            if (bid_ladder.contains(order.price)) {
                bid_ladder[order.price].size += order.size;
                this->trim_to_depth();
                return true;
            }
            bid_ladder[order.price].size = order.size;
            this->trim_to_depth();
            return true;
        }
    }
    if (order.action == Action::CANCEL) {
        if (order.side == Side::ASK) {
            if (ask_ladder.contains(order.price)) {
                if (ask_ladder[order.price].size < order.size) {
                    return false;
                }

                ask_ladder[order.price].size -= order.size;
                if (ask_ladder[order.price].size == 0) {
                    ask_ladder.erase(order.price);
                }
                this->trim_to_depth();
                return true;
            }
            return false;
        }
        if (order.side == Side::BID) {
            if (bid_ladder.contains(order.price)) {
                if (bid_ladder[order.price].size < order.size) {
                    return false;
                }

                bid_ladder[order.price].size -= order.size;
                if (bid_ladder[order.price].size == 0) {
                    bid_ladder.erase(order.price);
                }
                this->trim_to_depth();
                return true;
            }
            return false;
        }
    }
    if (order.action == Action::TRADE) {
        if (order.side == Side::ASK) {
            double unfilled = order.size;
            double take;
            for (auto it = ask_ladder.begin(); it != ask_ladder.end() && unfilled > 0.0;) {
                take = std::min(unfilled, it->second.size);
                unfilled -= take;
                it->second.size -= take;
                if (it->second.size <= 0.0) {
                    it = ask_ladder.erase(it);
                } else {
                    ++it;
                }
            }
            this->trim_to_depth();
        }
        if (order.side == Side::BID) {
            double unfilled = order.size;
            double take;
            for (auto it = bid_ladder.begin(); it != bid_ladder.end() && unfilled > 0.0;) {
                take = std::min(unfilled, it->second.size);
                unfilled -= take;
                it->second.size -= take;
                if (it->second.size <= 0.0) {
                    it = bid_ladder.erase(it);
                } else {
                    ++it;
                }
            }
            this->trim_to_depth();
        }
        return true;
    }

    return false;
}

// Double-dispatch entry
void Market::on_event(Event* event) {
    if (!event) return;
    event->dispatchTo(*this);
}

// Handlers
void Market::handle(HeartBeatEvent& hb) {
    this->check_staleness(hb);
}

void Market::handle(MarketEvent& /*ev*/) {
    // Default behavior: no-op. Specific event subtypes are handled by overloads.
}

void Market::handle(MarketByPriceEvent& mbp_event) {
    this->last_reception_timestamp = mbp_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = mbp_event.get_last_market_timestamp().order_gateway_in_timestamp;
    snapshot_to_ladder(mbp_event.get_snapshot_data(), this->bid_ladder, this->ask_ladder, static_cast<size_t>(this->depth));
    this->trim_to_depth();
    this->valid = check_snapshot();
}

// Specific handler for OrderBookSnapshotEvent
void Market::handle(SnapshotEvent& mbp_event) {
    this->last_reception_timestamp = mbp_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = mbp_event.get_last_market_timestamp().order_gateway_in_timestamp;
    snapshot_to_ladder(mbp_event.get_snapshot_data(), this->bid_ladder, this->ask_ladder, static_cast<size_t>(this->depth));
    this->trim_to_depth();
    this->valid = check_snapshot();
}

void Market::handle(OrderEvent& mbo_event) {
    this->last_reception_timestamp = mbo_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = mbo_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->valid = this->match(mbo_event.get_order());
}

void Market::handle(UpdateEvent& update_event) {
    this->last_reception_timestamp = update_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = update_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->update(update_event.get_update().level, update_event.get_update().side, update_event.get_update().action);
    this->valid = check_snapshot();
}

void Market::handle(UpdateBatchEvent& update_batch_event) {
    this->last_reception_timestamp = update_batch_event.get_reception_timestamp();

    const auto& messages = update_batch_event.get_messages();
    if (messages.empty()) {
        return;
    }

    for (const auto& message : messages) {
        this->last_order_gateway_in_timestamp = message.market_time_stamp.order_gateway_in_timestamp;
        this->update(message.update.level, message.update.side, message.update.action);
    }

    this->valid = check_snapshot();
}

void Market::trim_to_depth() {
    const size_t target_depth = this->depth > 0 ? static_cast<size_t>(this->depth) : 1u;

    while (this->bid_ladder.size() > target_depth) {
        auto it = this->bid_ladder.end();
        --it;
        this->bid_ladder.erase(it);
    }

    while (this->ask_ladder.size() > target_depth) {
        auto it = this->ask_ladder.end();
        --it;
        this->ask_ladder.erase(it);
    }
}

bool Market::check_snapshot() {
  if (this->ask_ladder.empty() || this->bid_ladder.empty()){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("snapshot received @ {} by {} has empty side(s), setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name));
    }
    return false;
  }

  if (this->get_best_ask_price() == 0){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("ask price received @ {} by {} is 0, setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name));
    }
    return false;
  }

  if (this->get_best_bid_price() == 0){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("bid price received @ {} by {} is 0, setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name));
    }
    return false;
  }

  if (this->get_best_ask_size() == 0){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("ask size received @ {} by {} is 0, setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name));
    }
    return false;
  }

  if (this->get_best_bid_size() == 0){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("bid size received @ {} by {} is 0, setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name));
    }
    return false;
  }

  if (this->get_best_bid_price() >= this->get_best_ask_price()){
    if (this->logger) {
      this->logger->log_error("Market", fmt::format("snapshot received @ {} by {} show bid={}>=ask={} , setting to invalid", Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()), this->name, this->get_best_bid_price(), this->get_best_ask_price()));
    }
    return false;
  }

  return true;
}
