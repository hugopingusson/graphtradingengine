//
// Created by hugo on 25/03/25.
//
#ifndef MARKET_H
#define MARKET_H

#include <array>
#include <cstddef>
#include <fmt/core.h>
#include <vector>

#include "../../Graph/Event.h"
#include "../../../Data/DataStructure/DataStructure.h"
#include "Node.h"


using namespace std;
using namespace fmt;


class Market:public virtual Producer {
    public:
    Market();
    ~Market() override = default;
    Market(const string& instrument,const string& exchange,const int& depth,const double& tick_value);

    const string& get_instrument() const;
    const string& get_exchange() const;

    double get_best_ask_price() const;
    double get_best_bid_price() const;
    double get_best_ask_size() const;
    double get_best_bid_size() const;
    std::size_t get_ask_level_count() const;
    std::size_t get_bid_level_count() const;

    bool match(Order order);
    void update(BookLevel level,Side side,Action action);

    int get_depth() const;
    double get_tick_value() const;

    bool check_snapshot();
    bool check_staleness(HeartBeatEvent& hb);

    void on_event(Event* event) override; // entry point
    // handlers used by double-dispatch
    void handle(MarketEvent& event);
    void handle(MarketByPriceEvent& event);
    void handle(SnapshotEvent& event);
    void handle(OrderEvent& event);
    void handle(UpdateEvent& event);
    void handle(UpdateBatchEvent& event);
    void handle(HeartBeatEvent& event);

    double ask_price(const std::size_t& i) const;
    double bid_price(const std::size_t& i) const;

    double ask_size(const std::size_t& i) const;
    double bid_size(const std::size_t& i) const;

    double mid() const;
    double bary() const;
    double spread() const;
    double imbalance() const;

    protected:
    void trim_to_depth();
    void reset_ladder();
    void load_snapshot(const SnapshotData& snapshot);

    int find_ask_index(const double& price) const;
    int find_bid_index(const double& price) const;
    std::size_t find_ask_insert_pos(const double& price) const;
    std::size_t find_bid_insert_pos(const double& price) const;
    void erase_ask_level(const std::size_t& idx);
    void erase_bid_level(const std::size_t& idx);
    void apply_ask_level(const double& price, const double& size, const int& count, const Action& action);
    void apply_bid_level(const double& price, const double& size, const int& count, const Action& action);

    string instrument;
    string exchange;
    std::array<BookLevel, kBookLevels> ask_levels;
    std::array<BookLevel, kBookLevels> bid_levels;
    std::size_t ask_level_count;
    std::size_t bid_level_count;
    int depth;
    double tick_value;
    bool snapshot_validation_enabled;
};


#endif //MARKET_H
