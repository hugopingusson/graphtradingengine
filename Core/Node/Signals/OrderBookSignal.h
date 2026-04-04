//
// Created by hugo on 01/04/25.
//

#ifndef SIMPLE_H
#define SIMPLE_H

#include "../Base/Node.h"
#include "../Base/MarketNode.h"

class Mid:public SingleInputConsumer {
    public:
    Mid();
    ~Mid() override = default;
    Mid(MarketOrderBook* market);

    void compute() override;

    protected:
    MarketOrderBook* market;

};


class Bary:public SingleInputConsumer {
    public:
    Bary();
    ~Bary() override = default;
    Bary(MarketOrderBook* market);

    void compute() override;

    protected:
    MarketOrderBook* market;

};



class Vwap:public SingleInputConsumer {
    public:
    Vwap();
    ~Vwap() override = default;
    Vwap(MarketOrderBook* market,double const& size);

    void compute() override;

    double get_ask_vwap() const;
    double get_bid_vwap() const;

    protected:
    MarketOrderBook* market;
    double amount;
    double ask_vwap;
    double bid_vwap;

};

class TopOfBookImbalance : public SingleInputConsumer {
public:
    TopOfBookImbalance();
    ~TopOfBookImbalance() override = default;
    TopOfBookImbalance(MarketOrderBook* market);

    void compute() override;

protected:
    MarketOrderBook* market;
};




#endif //SIMPLE_H
