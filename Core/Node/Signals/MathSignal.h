//
// Created by hugo on 01/04/25.
//

#ifndef MATHNODE_H
#define MATHNODE_H
#include "../Base/Node.h"
#include <string>

// class Skew:public Consumer {
//     public:
//     Skew();
//     ~Skew() override = default;
//     Skew(Signal* parent1,Signal* parent2);
//     void compute() override;
//     void update() override;
//
//     protected:
//     Signal* parent_1;
//     Signal* parent_2;
// };

class Print : public SingleInputConsumer {
public:
    Print();
    ~Print() override = default;
    Print(SingleInputConsumer* parent, const std::string& label = "Print");

    void compute() override;

private:
    SingleInputConsumer* parent_signal;
    std::string label;
};


#endif //MATHNODE_H
