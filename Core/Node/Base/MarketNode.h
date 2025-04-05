//
// Created by hugo on 25/03/25.
//
#ifndef MARKET_H
#define MARKET_H

#include <fmt/core.h>
#include <map>
#include <vector>
#include "../../Graph/Event.h"
#include "Node.h"


using namespace std;
using namespace fmt;

class Market:public SourceNode {

public:
    virtual ~Market() = default;
    Market();
    Market(const string& name,const string& instrument,const string& exchange);

    string get_instrument();
    string get_exchange();

protected:
    string instrument;
    string exchange;


};


class MarketTrade:public Market {
public:
    MarketTrade();
    ~MarketTrade() override = default;
    MarketTrade(const string& instrument,const string& exchange);

    int get_side();
    double get_trade_price();
    double get_base_quantity();
    double get_quote_quantity();

    bool check_trade();
    void update(Trade* trade);

protected:
    int side;
    double trade_price;
    double base_quantity;


};



class MarketOrderBook:public Market {
    public:
    MarketOrderBook();
    ~MarketOrderBook() override = default;
    MarketOrderBook(const string& instrument,const string& exchange,const int& depth,const double& tick_value);

    map<string,vector<double>> get_data();
    int get_depth();
    double get_tick_value();

    bool check_snapshot();
    void update(OrderBookSnapshot* order_book_snapshot);

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
    map<string,vector<double>> data;
    int depth;
    double tick_value;





};

#endif //MARKET_H
