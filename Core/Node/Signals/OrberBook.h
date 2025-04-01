//
// Created by hugo on 01/04/25.
//

#ifndef SIMPLE_H
#define SIMPLE_H

#include "../Node.h"
#include "../Market.h"

class Mid:public MonoValueNode {
    public:
    Mid();
    Mid(Market* market);

    double compute();

    protected:
    Market* parent;
};


class Bary:public MonoValueNode {
public:
    Bary();
    Bary(Market* market);

    double compute();

protected:
    Market* parent;
};


#endif //SIMPLE_H
