//
// Created by hugo on 01/04/25.
//

#ifndef SIMPLE_H
#define SIMPLE_H

#include "../Base/Node.h"
#include "../Base/MarketNode.h"

class Mid:public MonoSignal {
    public:
    Mid();
    ~Mid() override = default;
    Mid(MarketOrderBook* market);

    double compute()override;


};


class Bary:public MonoSignal {
    public:
    Bary();
    ~Bary() override = default;
    Bary(MarketOrderBook* market);

    double compute() override;

};



class Vwap:public Quote {
    public:
    Vwap();
    ~Vwap() override = default;
    Vwap(MarketOrderBook* market,double const& size);

    void update() override;

    protected:
    MarketOrderBook* market;
    double size;

};




#endif //SIMPLE_H
