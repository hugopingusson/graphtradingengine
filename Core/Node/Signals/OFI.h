//
// Created by hugo on 29/04/2026.
//

#ifndef GRAPHTRADINGENGINE_OFI_H
#define GRAPHTRADINGENGINE_OFI_H
#include "Base.h"
#include "../../../Data/DataStructure/Ladder.h"


class OFI: public MarketSignal {
    // 0: OFAsk ask order flow
    // 1: OFBid bid order flow
    // 2: OFAsk - AFBid
public:
    OFI();
    OFI(const string& instrument, const string& exchange);

    bool recompute() override;
    
private:
    Ladder last_ask_ladder;
    Ladder last_bid_ladder;

};


#endif //GRAPHTRADINGENGINE_OFI_H
