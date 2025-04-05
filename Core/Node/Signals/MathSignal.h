//
// Created by hugo on 01/04/25.
//

#ifndef MATHNODE_H
#define MATHNODE_H
#include "../Base/Node.h"

class Skew:public Signal{
    public:
    Skew();
    ~Skew() override = default;
    Skew(Signal* parent1,Signal* parent2);
    double compute() override;
    void update() override;

    protected:
    Signal* parent_1;
    Signal* parent_2;
};


#endif //MATHNODE_H
