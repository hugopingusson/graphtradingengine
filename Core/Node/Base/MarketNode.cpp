//
// Created by hugo on 25/03/25.
//

#include "MarketNode.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "../../../Helper/SaphirManager.h"

namespace {
bool is_valid_price_size(const double& price, const double& size) {
    return std::isfinite(price) && std::isfinite(size) && price > 0.0 && size > 0.0;
}
}

Market::Market()
    : instrument(),
      exchange(),
      ask_levels(),
      bid_levels(),
      ask_level_count(0),
      bid_level_count(0),
      depth(),
      tick_value(),
      snapshot_validation_enabled(false) {}

Market::Market(const string& instrument, const string& exchange, const int& depth, const double& tick_value)
    : Node(fmt::format("Market(instrument={},exchange={})", instrument, exchange)),
      instrument(instrument),
      exchange(exchange),
      ask_levels(),
      bid_levels(),
      ask_level_count(0),
      bid_level_count(0),
      depth(depth),
      tick_value(tick_value),
      snapshot_validation_enabled(false) {
    const SaphirManager saphir;
    if (this->depth <= 0) {
        this->depth = static_cast<int>(saphir.get_market_depth());
    }
    if (this->depth <= 0) {
        throw std::runtime_error(fmt::format(
            "Invalid market depth={} for Market(instrument={},exchange={})",
            this->depth,
            this->instrument,
            this->exchange
        ));
    }
    this->snapshot_validation_enabled = (saphir.get_logger_mode() == "debug");
    this->trim_to_depth();
}

const string& Market::get_instrument() const {
    return this->instrument;
}

const string& Market::get_exchange() const {
    return this->exchange;
}

double Market::get_tick_value() const {
    return this->tick_value;
}

int Market::get_depth() const {
    return this->depth;
}

std::size_t Market::get_ask_level_count() const {
    return this->ask_level_count;
}

std::size_t Market::get_bid_level_count() const {
    return this->bid_level_count;
}

double Market::ask_price(const std::size_t& i) const {
    if (i >= this->ask_level_count) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return this->ask_levels[i].price;
}

double Market::bid_price(const std::size_t& i) const {
    if (i >= this->bid_level_count) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return this->bid_levels[i].price;
}

double Market::ask_size(const std::size_t& i) const {
    if (i >= this->ask_level_count) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return this->ask_levels[i].size;
}

double Market::bid_size(const std::size_t& i) const {
    if (i >= this->bid_level_count) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return this->bid_levels[i].size;
}

double Market::get_best_ask_price() const {
    if (this->ask_level_count == 0) {
        return 0.0;
    }
    return this->ask_levels[0].price;
}

double Market::get_best_bid_price() const {
    if (this->bid_level_count == 0) {
        return 0.0;
    }
    return this->bid_levels[0].price;
}

double Market::get_best_ask_size() const {
    if (this->ask_level_count == 0) {
        return 0.0;
    }
    return this->ask_levels[0].size;
}

double Market::get_best_bid_size() const {
    if (this->bid_level_count == 0) {
        return 0.0;
    }
    return this->bid_levels[0].size;
}

double Market::mid() const {
    return 0.5 * (this->get_best_ask_price() + this->get_best_bid_price());
}

double Market::spread() const {
    return this->get_best_ask_price() - this->get_best_bid_price();
}

double Market::imbalance() const {
    const double ask = this->get_best_ask_size();
    const double bid = this->get_best_bid_size();
    const double denom = ask + bid;
    if (denom == 0.0) {
        return 0.0;
    }
    return (ask - bid) / denom;
}

double Market::bary() const {
    return this->mid() + 0.5 * this->spread() * this->imbalance();
}

bool Market::check_staleness(HeartBeatEvent& hb) {
    static constexpr int64_t kStaleThresholdNs = 3'000'000'000LL;
    if (hb.get_reception_timestamp() - this->last_reception_timestamp > kStaleThresholdNs) {
        if (this->logger) {
            this->logger->log_info(
                "Market",
                fmt::format(
                    "{} is now stale, setting to invalid, last reception was 3 sec ago at {}",
                    this->name,
                    Timestamp::unix_to_string(this->last_reception_timestamp)
                )
            );
        }
        this->valid = false;
        return false;
    }
    return true;
}

int Market::find_ask_index(const double& price) const {
    for (std::size_t i = 0; i < this->ask_level_count; ++i) {
        if (this->ask_levels[i].price == price) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int Market::find_bid_index(const double& price) const {
    for (std::size_t i = 0; i < this->bid_level_count; ++i) {
        if (this->bid_levels[i].price == price) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::size_t Market::find_ask_insert_pos(const double& price) const {
    std::size_t pos = 0;
    while (pos < this->ask_level_count && this->ask_levels[pos].price < price) {
        ++pos;
    }
    return pos;
}

std::size_t Market::find_bid_insert_pos(const double& price) const {
    std::size_t pos = 0;
    while (pos < this->bid_level_count && this->bid_levels[pos].price > price) {
        ++pos;
    }
    return pos;
}

void Market::erase_ask_level(const std::size_t& idx) {
    if (idx >= this->ask_level_count) {
        return;
    }
    for (std::size_t i = idx + 1; i < this->ask_level_count; ++i) {
        this->ask_levels[i - 1] = this->ask_levels[i];
    }
    --this->ask_level_count;
    this->ask_levels[this->ask_level_count] = BookLevel{};
}

void Market::erase_bid_level(const std::size_t& idx) {
    if (idx >= this->bid_level_count) {
        return;
    }
    for (std::size_t i = idx + 1; i < this->bid_level_count; ++i) {
        this->bid_levels[i - 1] = this->bid_levels[i];
    }
    --this->bid_level_count;
    this->bid_levels[this->bid_level_count] = BookLevel{};
}

void Market::apply_ask_level(const double& price, const double& size, const int& count, const Action& action) {
    const int idx = this->find_ask_index(price);
    if (action == Action::CANCEL || !is_valid_price_size(price, size)) {
        if (idx >= 0) {
            this->erase_ask_level(static_cast<std::size_t>(idx));
        }
        return;
    }

    if (idx >= 0) {
        this->ask_levels[static_cast<std::size_t>(idx)] = BookLevel{price, size, price * size, count};
        return;
    }

    const std::size_t pos = this->find_ask_insert_pos(price);
    if (pos >= kBookLevels) {
        return;
    }

    const std::size_t old_count = this->ask_level_count;
    const std::size_t new_count = std::min<std::size_t>(kBookLevels, old_count + 1);
    for (std::size_t i = new_count; i > pos + 1; --i) {
        this->ask_levels[i - 1] = this->ask_levels[i - 2];
    }
    this->ask_levels[pos] = BookLevel{price, size, price * size, count};
    this->ask_level_count = new_count;
}

void Market::apply_bid_level(const double& price, const double& size, const int& count, const Action& action) {
    const int idx = this->find_bid_index(price);
    if (action == Action::CANCEL || !is_valid_price_size(price, size)) {
        if (idx >= 0) {
            this->erase_bid_level(static_cast<std::size_t>(idx));
        }
        return;
    }

    if (idx >= 0) {
        this->bid_levels[static_cast<std::size_t>(idx)] = BookLevel{price, size, price * size, count};
        return;
    }

    const std::size_t pos = this->find_bid_insert_pos(price);
    if (pos >= kBookLevels) {
        return;
    }

    const std::size_t old_count = this->bid_level_count;
    const std::size_t new_count = std::min<std::size_t>(kBookLevels, old_count + 1);
    for (std::size_t i = new_count; i > pos + 1; --i) {
        this->bid_levels[i - 1] = this->bid_levels[i - 2];
    }
    this->bid_levels[pos] = BookLevel{price, size, price * size, count};
    this->bid_level_count = new_count;
}

void Market::update(BookLevel level, Side side, Action action) {
    if (side == Side::BID) {
        this->apply_bid_level(level.price, level.size, level.count, action);
    } else if (side == Side::ASK) {
        this->apply_ask_level(level.price, level.size, level.count, action);
    }
    this->trim_to_depth();
}

bool Market::match(Order order) {
    if (!std::isfinite(order.size) || order.size <= 0.0) {
        return false;
    }

    if (order.action == Action::ADD) {
        if (!std::isfinite(order.price) || order.price <= 0.0) {
            return false;
        }
        if (order.side == Side::ASK) {
            const int idx = this->find_ask_index(order.price);
            if (idx >= 0) {
                this->ask_levels[static_cast<std::size_t>(idx)].size += order.size;
                this->ask_levels[static_cast<std::size_t>(idx)].amount =
                    this->ask_levels[static_cast<std::size_t>(idx)].price * this->ask_levels[static_cast<std::size_t>(idx)].size;
            } else {
                this->apply_ask_level(order.price, order.size, 0, Action::ADD);
            }
            this->trim_to_depth();
            return true;
        }
        if (order.side == Side::BID) {
            const int idx = this->find_bid_index(order.price);
            if (idx >= 0) {
                this->bid_levels[static_cast<std::size_t>(idx)].size += order.size;
                this->bid_levels[static_cast<std::size_t>(idx)].amount =
                    this->bid_levels[static_cast<std::size_t>(idx)].price * this->bid_levels[static_cast<std::size_t>(idx)].size;
            } else {
                this->apply_bid_level(order.price, order.size, 0, Action::ADD);
            }
            this->trim_to_depth();
            return true;
        }
    }

    if (order.action == Action::CANCEL) {
        if (!std::isfinite(order.price) || order.price <= 0.0) {
            return false;
        }
        if (order.side == Side::ASK) {
            const int idx = this->find_ask_index(order.price);
            if (idx < 0) {
                return false;
            }
            auto& level = this->ask_levels[static_cast<std::size_t>(idx)];
            if (level.size < order.size) {
                return false;
            }
            level.size -= order.size;
            level.amount = level.price * level.size;
            if (level.size <= 0.0) {
                this->erase_ask_level(static_cast<std::size_t>(idx));
            }
            this->trim_to_depth();
            return true;
        }
        if (order.side == Side::BID) {
            const int idx = this->find_bid_index(order.price);
            if (idx < 0) {
                return false;
            }
            auto& level = this->bid_levels[static_cast<std::size_t>(idx)];
            if (level.size < order.size) {
                return false;
            }
            level.size -= order.size;
            level.amount = level.price * level.size;
            if (level.size <= 0.0) {
                this->erase_bid_level(static_cast<std::size_t>(idx));
            }
            this->trim_to_depth();
            return true;
        }
    }

    if (order.action == Action::TRADE) {
        if (order.side == Side::ASK) {
            double unfilled = order.size;
            while (unfilled > 0.0 && this->ask_level_count > 0) {
                const double take = std::min(unfilled, this->ask_levels[0].size);
                this->ask_levels[0].size -= take;
                this->ask_levels[0].amount = this->ask_levels[0].price * this->ask_levels[0].size;
                unfilled -= take;
                if (this->ask_levels[0].size <= 0.0) {
                    this->erase_ask_level(0);
                }
            }
            this->trim_to_depth();
            return true;
        }
        if (order.side == Side::BID) {
            double unfilled = order.size;
            while (unfilled > 0.0 && this->bid_level_count > 0) {
                const double take = std::min(unfilled, this->bid_levels[0].size);
                this->bid_levels[0].size -= take;
                this->bid_levels[0].amount = this->bid_levels[0].price * this->bid_levels[0].size;
                unfilled -= take;
                if (this->bid_levels[0].size <= 0.0) {
                    this->erase_bid_level(0);
                }
            }
            this->trim_to_depth();
            return true;
        }
    }

    return false;
}

void Market::on_event(Event* event) {
    if (!event) {
        return;
    }

    this->clear_dirty();
    if (dynamic_cast<MarketEvent*>(event) != nullptr) {
        this->mark_dirty();
    }

    event->dispatchTo(*this);
}

void Market::handle(HeartBeatEvent& hb) {
    this->check_staleness(hb);
}

void Market::handle(MarketEvent& /*ev*/) {
    // Default behavior: no-op. Specific event subtypes are handled by overloads.
}

void Market::reset_ladder() {
    this->ask_level_count = 0;
    this->bid_level_count = 0;
    this->ask_levels.fill(BookLevel{});
    this->bid_levels.fill(BookLevel{});
}

void Market::load_snapshot(const SnapshotData& snapshot) {
    this->reset_ladder();
    const std::size_t target_depth = std::min<std::size_t>(
        kBookLevels,
        this->depth > 0 ? static_cast<std::size_t>(this->depth) : 1u
    );

    for (std::size_t i = 0; i < target_depth; ++i) {
        const double price = snapshot.bid_price[i];
        const double size = snapshot.bid_size[i];
        if (is_valid_price_size(price, size)) {
            this->bid_levels[this->bid_level_count++] = BookLevel{
                price,
                size,
                price * size,
                static_cast<int>(snapshot.bid_count[i])
            };
        }
    }

    for (std::size_t i = 0; i < target_depth; ++i) {
        const double price = snapshot.ask_price[i];
        const double size = snapshot.ask_size[i];
        if (is_valid_price_size(price, size)) {
            this->ask_levels[this->ask_level_count++] = BookLevel{
                price,
                size,
                price * size,
                static_cast<int>(snapshot.ask_count[i])
            };
        }
    }

    this->trim_to_depth();
}

void Market::handle(MarketByPriceEvent& mbp_event) {
    this->last_reception_timestamp = mbp_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = mbp_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->load_snapshot(mbp_event.get_snapshot_data());
    this->valid = this->snapshot_validation_enabled
        ? this->check_snapshot()
        : (this->ask_level_count > 0 && this->bid_level_count > 0);
}

void Market::handle(SnapshotEvent& mbp_event) {
    this->last_reception_timestamp = mbp_event.get_reception_timestamp();
    this->last_order_gateway_in_timestamp = mbp_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->load_snapshot(mbp_event.get_snapshot_data());
    this->valid = this->snapshot_validation_enabled
        ? this->check_snapshot()
        : (this->ask_level_count > 0 && this->bid_level_count > 0);
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
    this->valid = this->snapshot_validation_enabled
        ? this->check_snapshot()
        : (this->ask_level_count > 0 && this->bid_level_count > 0);
}

void Market::handle(UpdateBatchEvent& update_batch_event) {
    this->last_reception_timestamp = update_batch_event.get_reception_timestamp();
    const auto& messages = update_batch_event.get_messages();
    if (messages.empty()) {
        return;
    }

    for (const auto& message : messages) {
        this->last_order_gateway_in_timestamp = message.market_time_stamp.order_gateway_in_timestamp;
        const auto& update = message.update;
        if (update.side == Side::BID) {
            this->apply_bid_level(update.level.price, update.level.size, update.level.count, update.action);
        } else if (update.side == Side::ASK) {
            this->apply_ask_level(update.level.price, update.level.size, update.level.count, update.action);
        }
    }

    this->trim_to_depth();
    this->valid = this->snapshot_validation_enabled
        ? this->check_snapshot()
        : (this->ask_level_count > 0 && this->bid_level_count > 0);
}

void Market::trim_to_depth() {
    const std::size_t target_depth = std::min<std::size_t>(
        kBookLevels,
        this->depth > 0 ? static_cast<std::size_t>(this->depth) : 1u
    );
    if (this->bid_level_count > target_depth) {
        this->bid_level_count = target_depth;
    }
    if (this->ask_level_count > target_depth) {
        this->ask_level_count = target_depth;
    }
}

bool Market::check_snapshot() {
    if (this->ask_level_count == 0 || this->bid_level_count == 0) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "snapshot received @ {} by {} has empty side(s), setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name
                )
            );
        }
        return false;
    }

    if (this->get_best_ask_price() <= 0.0 || !std::isfinite(this->get_best_ask_price())) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "ask price received @ {} by {} is invalid, setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name
                )
            );
        }
        return false;
    }

    if (this->get_best_bid_price() <= 0.0 || !std::isfinite(this->get_best_bid_price())) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "bid price received @ {} by {} is invalid, setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name
                )
            );
        }
        return false;
    }

    if (this->get_best_ask_size() <= 0.0 || !std::isfinite(this->get_best_ask_size())) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "ask size received @ {} by {} is invalid, setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name
                )
            );
        }
        return false;
    }

    if (this->get_best_bid_size() <= 0.0 || !std::isfinite(this->get_best_bid_size())) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "bid size received @ {} by {} is invalid, setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name
                )
            );
        }
        return false;
    }

    if (this->get_best_bid_price() >= this->get_best_ask_price()) {
        if (this->logger) {
            this->logger->log_error(
                "Market",
                fmt::format(
                    "snapshot received @ {} by {} show bid={}>=ask={} , setting to invalid",
                    Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),
                    this->name,
                    this->get_best_bid_price(),
                    this->get_best_ask_price()
                )
            );
        }
        return false;
    }

    return true;
}
