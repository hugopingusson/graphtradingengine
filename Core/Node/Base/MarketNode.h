//
// Created by hugo on 25/03/25.
//
#ifndef MARKET_H
#define MARKET_H

#include <fmt/core.h>
#include <map>
#include <vector>

#include "../../Graph/Event.h"
#include "../Base/HeartBeat.h"
#include "../../../Data/DataStructure/DataStructure.h"
#include "Node.h"


using namespace std;
using namespace fmt;


class Market:public virtual Producer {
// class Market:public virtual SourceNode<Market> {

public:
    virtual ~Market() = default;
    Market();
    Market(const string& instrument,const string& exchange); // No only default


    string get_instrument();
    string get_exchange();

protected:
    string instrument;
    string exchange;


};




class MarketOrderBook:public Market  {
    public:
    MarketOrderBook();
    ~MarketOrderBook() override = default;
    MarketOrderBook(const string& instrument,const string& exchange,const int& depth,const double& tick_value);
    // MarketOrderBook(const string& instrument,const string& exchange,const int& depth,const double& tick_value,HeartBeat* heart_beat_node);

    const AskLadder& get_ask_ladder() const;
    const BidLadder& get_bid_ladder() const;


    double get_best_ask_price() const;
    double get_best_bid_price() const;
    double get_best_ask_size() const;
    double get_best_bid_size() const;

    bool match(Order order);
    void update(BookLevel level,Side side,Action action);


    int get_depth();
    double get_tick_value();

    bool check_snapshot();
    bool check_staleness(HeartBeatEvent& hb);

    void on_event(Event* event) override; // entry point
    // handlers used by double-dispatch
    void handle(MarketEvent& event);
    void handle(MarketByPriceEvent& event);
    void handle(SnapshotEvent& event);
    void handle(OrderEvent& event);
    void handle(UpdateEvent& event);
    void handle(HeartBeatEvent& event);

    double ask_price(const std::size_t& i) const;
    double bid_price(const std::size_t& i) const;

    double ask_size(const std::size_t& i) const;
    double bid_size(const std::size_t& i) const;

    double mid() const;
    double bary() const;
    double spread() const;
    double imbalance() const;

    // double bid_amount(const int& i) const;
    // double ask_amount(const int& i) const;

    // double bid_amount_at_mid(const int& i) const;
    // double ask_amount_at_mid(const int& i) const;

    // double cumulative_ask_size(const int& i) const;
    // double cumulative_bid_size(const int& i) const;

    // double cumulative_ask_amount(const int& i) const;
    // double cumulative_bid_amount(const int& i) const;




    protected:
    AskLadder ask_ladder;
    BidLadder bid_ladder;
    int depth;
    double tick_value;


};

//
// class MarketTrade:public Market{
// public:
//     MarketTrade();
//     ~MarketTrade() override = default;
//     MarketTrade(const string& instrument,const string& exchange);
//
//     Side get_side();
//     double get_trade_price();
//     double get_base_quantity();
//     double get_quote_quantity();
//
//     bool check_trade();
//     // void handle(TradeEvent& trade) override;
//     void on_event(Event* event) override;
//
//
// protected:
//     Side side;
//     double trade_price;
//     double base_quantity;
//
//
// };


#endif //MARKET_H
