//
// Created by hugo on 25/03/25.
//
#ifndef MARKET_H
#define MARKET_H

#include <spdlog/fmt/fmt.h>
#include "Node.h"

using namespace std;
using namespace fmt;
#include <map>
#include <vector>

class Market:public Node {
    public:
    Market();
    Market(const string& instrument,const string& exchange,const int& depth,const double& tick_value);

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
    map<string,vector<double>> data;
    string instrument;
    string exchange;
    int depth;
    double tick_value;





};



#endif //MARKET_H
