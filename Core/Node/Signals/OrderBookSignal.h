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

    double compute();

    protected:
    MarketOrderBook* parent;
};


class Bary:public MonoSignal {
public:
    Bary();
    ~Bary() override = default;
    Bary(MarketOrderBook* market);

    double compute();

protected:
    MarketOrderBook* parent;
};


#endif //SIMPLE_H
