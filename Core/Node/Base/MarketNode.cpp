//
// Created by hugo on 25/03/25.
//

#include "MarketNode.h"

#include <limits>

Market::Market(){}
Market::Market(const string& instrument, const string& exchange):instrument(instrument),exchange(exchange) {}


string Market::get_instrument() {
    return this->instrument;
}

string Market::get_exchange() {
    return this->exchange;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

MarketOrderBook::MarketOrderBook():depth(),tick_value(){}
MarketOrderBook::MarketOrderBook(const string &instrument, const string& exchange, const int& depth, const double& tick_value):Node(fmt::format("MarketOrderBook(instrument={},exchange={})",instrument,exchange)),Market(instrument,exchange),depth(depth),tick_value(tick_value){}


const AskLadder& MarketOrderBook::get_ask_ladder() const {
    return this->ask_ladder;
}

const BidLadder& MarketOrderBook::get_bid_ladder() const {
    return this->bid_ladder;
}

double MarketOrderBook::get_tick_value() {
    return this->tick_value;
}

int MarketOrderBook::get_depth() {
    return this->depth;
}


double MarketOrderBook::ask_price(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->ask_ladder.begin(); it != this->ask_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->first;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double MarketOrderBook::bid_price(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->bid_ladder.begin(); it != this->bid_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->first;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double MarketOrderBook::ask_size(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->ask_ladder.begin(); it != this->ask_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->second.size;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double MarketOrderBook::bid_size(const std::size_t& i) const {
    std::size_t idx = 0;
    for (auto it = this->bid_ladder.begin(); it != this->bid_ladder.end(); ++it, ++idx) {
        if (idx == i) {
            return it->second.size;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}


double MarketOrderBook::get_best_ask_price() const {
    if (ask_ladder.empty()) {
        return 0.0;
    }
    return ask_ladder.begin()->first;
}
double MarketOrderBook::get_best_bid_price() const {
    if (bid_ladder.empty()) {
        return 0.0;
    }
    return bid_ladder.begin()->first;
}

double MarketOrderBook::get_best_ask_size() const {
    if (ask_ladder.empty()) {
        return 0.0;
    }
    return ask_ladder.begin()->second.size;
}
double MarketOrderBook::get_best_bid_size() const {
    if (bid_ladder.empty()) {
        return 0.0;
    }
    return bid_ladder.begin()->second.size;
}

double MarketOrderBook::mid() const {
    return 0.5*(this->get_best_ask_price()+this->get_best_bid_price());
}

double MarketOrderBook::spread() const {
    return this->get_best_ask_price()-this->get_best_bid_price();
}

double MarketOrderBook::imbalance() const {
    const double ask_size = this->get_best_ask_size();
    const double bid_size = this->get_best_bid_size();
    const double denom = ask_size + bid_size;
    if (denom == 0.0) {
        return 0.0;
    }
    return (ask_size - bid_size) / denom;
}

double MarketOrderBook::bary() const {
    return this->mid()+0.5*this->spread()*this->imbalance();
}




bool MarketOrderBook::check_staleness(HeartBeatEvent& hb) {
    static constexpr int64_t kStaleThresholdNs = 3'000'000'000LL;
    if (hb.get_streamer_in_timestamp() - this->last_streamer_in_timestamp > kStaleThresholdNs) {
        if (this->logger) {
            this->logger->log_info("MarketOrderBook", fmt::format("{} is now stale, setting to invalid, last streamer in was 3 sec ago at {}", this->name, Timestamp::unix_to_string(this->last_streamer_in_timestamp)));
        }
        this->valid=false;
        return false;
    }
    return true;
}


void MarketOrderBook::update(BookLevel level, Side side, Action action) {
    if (side==Side::BID) {
        if (action==Action::CANCEL) {
            bid_ladder.erase(level.price);
        }
        if (action==Action::ADD || action==Action::MODIFY) {
            bid_ladder[level.price]=level;
        }
    }
    if (side==Side::ASK) {
        if (action==Action::CANCEL) {
            ask_ladder.erase(level.price);
        }
        if (action==Action::ADD || action==Action::MODIFY) {
            ask_ladder[level.price]=level;
        }
    }
}

bool MarketOrderBook::match(Order order) {
    if (order.action==Action::ADD) {
        if (order.side==Side::ASK) {
            if (ask_ladder.contains(order.price)){
                ask_ladder[order.price].size+=order.size;
                return true;
            }
            else {
                ask_ladder[order.price].size=order.size;
                return true;
            }
        }
        if (order.side==Side::BID) {
            if (bid_ladder.contains(order.price)){
                bid_ladder[order.price].size+=order.size;
                return true;
            }
            else {
                bid_ladder[order.price].size=order.size;
                return true;
            }
        }

    }
    if (order.action==Action::CANCEL) {
        if (order.side==Side::ASK) {
            if (ask_ladder.contains(order.price)){

                if (ask_ladder[order.price].size<order.size) {
                    return false;
                }

                ask_ladder[order.price].size-=order.size;
                if (ask_ladder[order.price].size==0) {
                    ask_ladder.erase(order.price);
                }
                return true;
            }
            else {
                return false;
            }
        }
        if (order.side==Side::BID) {
            if (bid_ladder.contains(order.price)){

                if (bid_ladder[order.price].size<order.size) {
                    return false;
                }

                bid_ladder[order.price].size-=order.size;
                if (bid_ladder[order.price].size==0) {
                    bid_ladder.erase(order.price);
                }
                return true;
            }
            else {
                return false;
            }
        }

    }
    if (order.action==Action::TRADE) {
        if (order.side==Side::ASK) {
            double unfilled=order.size;
            double take;
            for (auto it = ask_ladder.begin(); it != ask_ladder.end() && unfilled > 0.0;) {
                take=std::min(unfilled,it->second.size);
                unfilled-=take;
                it->second.size-=take;
                if (it->second.size<=0.0) {
                    it = ask_ladder.erase(it);
                } else {
                    ++it;
                }
            }
        }
        if (order.side==Side::BID) {
            double unfilled=order.size;
            double take;
            for (auto it = bid_ladder.begin(); it != bid_ladder.end() && unfilled > 0.0;) {
                take=std::min(unfilled,it->second.size);
                unfilled-=take;
                it->second.size-=take;
                if (it->second.size<=0.0) {
                    it = bid_ladder.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return true;

    }

    return false;
}


// Double-dispatch entry
void MarketOrderBook::on_event(Event* event) {
    if (!event) return;
    event->dispatchTo(*this);
}

// Handlers
void MarketOrderBook::handle(HeartBeatEvent& hb) {
    this->check_staleness(hb);
}

void MarketOrderBook::handle(MarketEvent& /*ev*/) {
    // Par défaut, ne rien faire. Les sous-types spécifiques traiteront.
}

// Specific handler for OrderBookSnapshotEvent
void MarketOrderBook::handle(MBPEvent& mbp_event) {
    this->last_streamer_in_timestamp = mbp_event.get_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = mbp_event.get_capture_server_in_timestamp();
    this->last_order_gateway_in_timestamp = mbp_event.get_last_market_timestamp().order_gateway_in_timestamp;
    snapshot_to_ladder(mbp_event.get_snapshot_data(),this->bid_ladder,this->ask_ladder);
    this->valid=check_snapshot();
}

void MarketOrderBook::handle(MBOEvent& mbo_event) {
    this->last_streamer_in_timestamp = mbo_event.get_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = mbo_event.get_capture_server_in_timestamp();
    this->last_order_gateway_in_timestamp = mbo_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->valid=this->match(mbo_event.get_order());
}

void MarketOrderBook::handle(UpdateEvent& update_event) {
    this->last_streamer_in_timestamp = update_event.get_streamer_in_timestamp();
    this->last_capture_server_in_timestamp = update_event.get_capture_server_in_timestamp();
    this->last_order_gateway_in_timestamp = update_event.get_last_market_timestamp().order_gateway_in_timestamp;
    this->update(update_event.get_update().level,update_event.get_update().side,update_event.get_update().action);
    this->valid=check_snapshot();
}




bool MarketOrderBook::check_snapshot() {
  if (this->ask_ladder.empty() || this->bid_ladder.empty()){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("snapshot received @ {} by {} has empty side(s), setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    }
    return false;
  }

  if (this->get_best_ask_price()==0){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("ask price received @ {} by {} is 0, setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    }
    return false;
  }

  if (this->get_best_bid_price()==0){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("bid price received @ {} by {} is 0, setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    }
    return false;
  }

  if (this->get_best_ask_size()==0){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("ask size received @ {} by {} is 0, setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    }
    return false;
  }

  if (this->get_best_bid_size()==0){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("bid size received @ {} by {} is 0, setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name));
    }
    return false;
  }

  if (this->get_best_bid_price()>=this->get_best_ask_price()){
    if (this->logger) {
      this->logger->log_error("MarketOrderBook",fmt::format("snapshot received @ {} by {} show bid={}>=ask={} , setting to invalid",Timestamp::unix_to_string(this->get_last_order_gateway_in_timestamp()),this->name,this->get_best_bid_price(),this->get_best_ask_price()));
    }
  	return false;
  }

  return true;

}
