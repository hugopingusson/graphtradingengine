//
// Created by hugo on 12/04/25.
//

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Event.h"

template <typename Derived>
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handle(Event& event) {}
    virtual void handle(MarketEvent& market_event) {}
    virtual void handle(OrderBookSnapshot& order_book_snapshot) {}
    virtual void handle(Trade& trade) {}
    virtual void handle(HeartBeatEvent& heart_beat_event) {}

    // Optionally, you can provide default handling for any event types
};


#endif //EVENTHANDLER_H
