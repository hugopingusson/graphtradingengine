//
// Created by hugo on 25/03/25.
//

#ifndef MARKETEVENT_H
#define MARKETEVENT_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../Data/DataStructure/DataStructure.h"

// Forward declaration to allow double-dispatch without circular include
class MarketOrderBook;

using namespace std;



class Event {
    public:
    Event();
    virtual ~Event() = default;
    explicit Event(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] int64_t get_last_streamer_in_timestamp() const;
    [[nodiscard]] int get_source_id_trigger() const;

    // Double dispatch entry point
    virtual void dispatchTo(MarketOrderBook& target) = 0;


    protected:
    int64_t last_streamer_in_timestamp;
    int source_id_trigger;


};

class HeartBeatEvent : public Event {
    public:
    HeartBeatEvent();
    ~HeartBeatEvent() override =default;
    explicit HeartBeatEvent(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency);

    double get_frequency() const;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    double frequency;
};



class MarketEvent : public Event {
    public:
    MarketEvent();
    ~MarketEvent() override =default;
    // MarketEvent(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);
    MarketEvent(const MarketTimeStamp& market_time_stamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const;

    void dispatchTo(MarketOrderBook& target) override;


    protected:

    MarketTimeStamp market_timestamp;

};



class OrderBookSnapshotEvent : public MarketEvent {
    public:
    OrderBookSnapshotEvent();
    ~OrderBookSnapshotEvent() override =default;
    OrderBookSnapshotEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const MarketByPriceMessage& mbp_message);

    [[nodiscard]] MarketByPriceMessage get_message() const;
    [[nodiscard]] SnapshotData get_snapshot_data() const;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    MarketByPriceMessage mbp_message;

};

class IncrementalEvent : public MarketEvent {
public:
    IncrementalEvent();
    ~IncrementalEvent() override =default;
    IncrementalEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const MarketByOrderMessage& mbo_message);

    [[nodiscard]] MarketByOrderMessage get_message() const;
    [[nodiscard]] ActionData get_action_data() const;

    void dispatchTo(MarketOrderBook& target) override;

protected:
    MarketByOrderMessage mbo_message;

};


class TradeEvent : public MarketEvent {
    public:
    TradeEvent();
    ~TradeEvent() override =default;
    TradeEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const Side& side,const double& trade_price,const double& base_quantity);

    Side get_side() const;
    double get_trade_price() const;
    double get_base_quantity() const;


    protected:
    Side side;
    double trade_price;
    double base_quantity;

};


#endif //MARKETEVENT_H
