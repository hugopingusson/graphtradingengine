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


class Market:public virtual SourceNode {
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




class MarketOrderBook:public Market,public ChildNode  {
    public:
    MarketOrderBook();
    ~MarketOrderBook() override = default;
    MarketOrderBook(const string& instrument,const string& exchange,const int& depth,const double& tick_value);
    MarketOrderBook(const string& instrument,const string& exchange,const int& depth,const double& tick_value,HeartBeat* heart_beat_node);

    OrderBookSnapshotData get_data();
    int get_depth();
    double get_tick_value();

    bool check_snapshot();
    void on_event(Event* event) override;
    // void handle(OrderBookSnapshotEvent& event) override;
    void update() override;

    double ask_price(const int& i) const;
    double ask_size(const int& i) const;
    double bid_price(const int& i) const;
    double bid_size(const int& i) const;

    double mid() const;
    double bary() const;
    double spread() const;
    double imbalance() const;

    double bid_amount(const int& i) const;
    double ask_amount(const int& i) const;

    double bid_amount_at_mid(const int& i) const;
    double ask_amount_at_mid(const int& i) const;

    double cumulative_ask_size(const int& i) const;
    double cumulative_bid_size(const int& i) const;

    double cumulative_ask_amount(const int& i) const;
    double cumulative_bid_amount(const int& i) const;




    protected:
    OrderBookSnapshotData data;
    int depth;
    double tick_value;
    HeartBeat* heart_beat_node;




};


class MarketTrade:public Market{
public:
    MarketTrade();
    ~MarketTrade() override = default;
    MarketTrade(const string& instrument,const string& exchange);

    int get_side();
    double get_trade_price();
    double get_base_quantity();
    double get_quote_quantity();

    bool check_trade();
    // void handle(TradeEvent& trade) override;
    void on_event(Event* event) override;


protected:
    int side;
    double trade_price;
    double base_quantity;


};


#endif //MARKET_H
