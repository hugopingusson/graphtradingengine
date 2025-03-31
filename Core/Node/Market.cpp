//
// Created by hugo on 25/03/25.
//

#include "Market.h"

#include <iostream>


Market::Market():Node(),instrument(string()),exchange(string()),depth(int()),tick_value(double()){};
Market::Market(const string &instrument, const string &exchange, const int &depth, const double &tick_value):Node(fmt::format("Market(instrument={},exchange={})",instrument,exchange)),instrument(instrument),exchange(exchange),depth(depth),tick_value(tick_value) {}

double Market::ask_size(const int& i) const {
    return data.at("ask_size").at(i);
}

double Market::ask_price(const int& i) const {
    return data.at("ask_price").at(i);
}

double Market::bid_size(const int& i) const {
    return data.at("bid_size").at(i);
}

double Market::bid_price(const int& i) const {
    return data.at("bid_price").at(i);
}


double Market::mid() const {
    return 0.5*(this->ask_price(0)+this->bid_price(0));
}

double Market::spread() const {
    return this->ask_price(0)-this->bid_price(0);
}

double Market::imbalance() const {
    return (this->ask_size(0)-this->bid_size(0))/(this->ask_size(0)-this->bid_size(0));
}

double Market::bary() const {
    return this->mid()+0.5*this->spread()*this->imbalance();
}

double Market::bid_amount(const int& i) const {
    return this->bid_size(i)*this->bid_price(i);
}
double Market::ask_amount(const int& i) const {
    return this->ask_size(i)*this->ask_price(i);
}

double Market::bid_amount_at_mid(const int& i) const {
    return this->bid_size(i)*this->mid();
}
double Market::ask_amount_at_mid(const int& i) const {
    return this->ask_size(i)*this->mid();
}

double Market::cumulative_ask_size(const int& i) const {
    double size=0;
    for (int k=0;k<=i;k++) {
        size+=this->ask_size(i);
    }

    return size;
}

double Market::cumulative_bid_size(const int& i) const {
    double size=0;
    for (int k=0;k<=i;k++) {
        size+=this->bid_size(i);
    }

    return size;
}

double Market::cumulative_ask_amount(const int& i) const {
    double amount=0;
    for (int k=0;k<=i;k++) {
        amount+=this->ask_amount(i);
    }

    return amount;
}

double Market::cumulative_bid_amount(const int& i) const {
    double amount=0;
    for (int k=0;k<=i;k++) {
        amount+=this->bid_amount(i);
    }

    return amount;
}



void Market::update() {
    cout<<"Market::update"<<endl;
}


