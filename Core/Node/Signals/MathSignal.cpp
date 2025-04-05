//
// Created by hugo on 01/04/25.
//

#include "MathSignal.h"
#include <fmt/format.h>


Skew::Skew():parent_1(nullptr),parent_2(nullptr) {}

Skew::Skew(Signal *parent1, Signal *parent2) {
    this->parent_1 = parent1;
    this->parent_2 = parent2;

    this->name=fmt::format("Skew({},{})",parent_1->get_name(),parent_2->get_name());

}


double Skew::compute() {
    return (this->parent_1->get_value()-this->parent_2->get_value())/this->parent_2->get_value();
}


void Skew::update() {
    if (parent_1->is_valid() & parent_2->is_valid()) {
        double new_value=this->compute();
        if (isnan(new_value)) {
            this->valid=false;
        }
        else {
            this->valid=true;
            this->value=new_value;
        }
    }
    else {
        this->valid=false;
    }
}