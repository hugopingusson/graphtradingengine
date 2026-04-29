//
// Created by hugo on 29/04/26.
//

#ifndef VWAP_H
#define VWAP_H

#include "Quote.h"

class Vwap : public MarketQuote {
public:
    Vwap();
    ~Vwap() override = default;
    explicit Vwap(const string& instrument, const string& exchange, double const& size);

    [[nodiscard]] double get_value() const;
    [[nodiscard]] double get_ask_vwap() const;
    [[nodiscard]] double get_bid_vwap() const;

protected:
    bool recompute() override;

    double value;
    double size;
    double ask_vwap;
    double bid_vwap;
};

#endif //VWAP_H
