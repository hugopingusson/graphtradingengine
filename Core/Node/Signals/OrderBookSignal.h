//
// Created by hugo on 01/04/25.
//

#ifndef SIMPLE_H
#define SIMPLE_H

#include "../Base/Node.h"
#include "../Base/MarketNode.h"

class Mid:public Consumer {
    public:
    Mid();
    ~Mid() override = default;
    Mid(MarketOrderBook* market);

    void compute() override;


};


class Bary:public Consumer {
    public:
    Bary();
    ~Bary() override = default;
    Bary(MarketOrderBook* market);

    void compute() override;

};



class Vwap:public Quote, public Consumer {
    public:
    Vwap();
    ~Vwap() override = default;
    Vwap(MarketOrderBook* market,double const& size);

    void compute() override;

    protected:
    MarketOrderBook* market;
    double size;

};




#endif //SIMPLE_H
