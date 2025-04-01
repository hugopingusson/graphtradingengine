//
// Created by hugo on 01/04/25.
//

#include "OrberBook.h"

#include <fmt/format.h>


Mid::Mid():parent(nullptr){
}

Mid::Mid(Market* market) {
    this->parent=market;
    this->name=fmt::format("Mid({}@{})",market->get_instrument(),market->get_exchange());
}

double Mid::compute() {
    return this->parent->mid();
}


Bary::Bary():parent(nullptr){
}

Bary::Bary(Market* market) {
    this->parent=market;
    this->name=fmt::format("Bary({}@{})",market->get_instrument(),market->get_exchange());
}

double Bary::compute() {
    return this->parent->bary();
}