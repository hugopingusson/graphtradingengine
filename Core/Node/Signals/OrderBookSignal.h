//
// Created by hugo on 01/04/25.
//

#ifndef SIMPLE_H
#define SIMPLE_H

#include "../Base/Node.h"
#include "../Base/MarketNode.h"

class Mid:public MarketConsumer {
    public:
    Mid();
    ~Mid() override = default;
    explicit Mid(const string& instrument, const string& exchange);

    bool recompute() override;
    [[nodiscard]] double get_value() const;

    protected:
    double value;
};


class Bary:public MarketConsumer {
    public:
    Bary();
    ~Bary() override = default;
    explicit Bary(const string& instrument, const string& exchange);

    bool recompute() override;
    [[nodiscard]] double get_value() const;

    protected:
    double value;
};
class TopOfBookImbalance : public MarketConsumer {
public:
    TopOfBookImbalance();
    ~TopOfBookImbalance() override = default;
    explicit TopOfBookImbalance(const string& instrument, const string& exchange);

    bool recompute() override;
    [[nodiscard]] double get_value() const;

protected:
    double value;
};




#endif //SIMPLE_H
