//
// Created by hugo on 01/04/25.
//

#ifndef MATHNODE_H
#define MATHNODE_H
#include "../Node.h"

class Skew:public ValueNode{
    public:
    Skew();
    Skew(ValueNode* parent1,ValueNode* parent2);
    double compute() override;
    void update() override;

    protected:
    ValueNode* parent_1;
    ValueNode* parent_2;
};


#endif //MATHNODE_H
