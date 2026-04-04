//
// Created by hugo on 01/04/25.
//

#include "MathSignal.h"
#include <cmath>
#include <iostream>
#include <fmt/format.h>


// Skew::Skew():parent_1(nullptr),parent_2(nullptr) {}
//
// Skew::Skew(Signal *parent1, Signal *parent2) {
//     this->parent_1 = parent1;
//     this->parent_2 = parent2;
//
//     this->name=fmt::format("Skew({},{})",parent_1->get_name(),parent_2->get_name());
//     this->mark_dirty();
//
// }
//
//
// double Skew::compute() {
//     return (this->parent_1->get_value()-this->parent_2->get_value())/this->parent_2->get_value();
// }
//
//
// void Skew::update() {
//     if (parent_1 && parent_2 && parent_1->is_valid() && parent_2->is_valid()) {
//         double new_value=this->compute();
//         if (isnan(new_value)) {
//             this->mark_dirty();
//         }
//         else {
//             this->value=new_value;
//             this->clear_dirty();
//         }
//     }
//     else {
//         this->mark_dirty();
//     }
// }

Print::Print() : SingleInputConsumer(), parent_signal(nullptr), label("Print") {}

Print::Print(SingleInputConsumer* parent, const std::string& label)
    : SingleInputConsumer(parent), parent_signal(parent), label(label) {
    if (this->parent_signal) {
        this->name = fmt::format("Print(parent={})", this->parent_signal->get_name());
    } else {
        this->name = "Print(parent=null)";
    }
    this->mark_dirty();
}

void Print::compute() {
    if (!this->parent_signal) {
        this->value = std::nan("");
        this->valid = false;
        return;
    }

    this->value = this->parent_signal->get_value();
    this->valid = !std::isnan(this->value);

    if (this->valid) {
        std::cout << this->label << ": " << this->value << std::endl;
    }
}
