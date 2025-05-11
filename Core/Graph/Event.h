//
// Created by hugo on 25/03/25.
//

#ifndef MARKETEVENT_H
#define MARKETEVENT_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../Data/DataStructure/DataStructure.h"

using namespace std;

// template <typename DerivedHandler>
// class EventHandler;


class Event {
    public:
    Event();
    virtual ~Event() = default;
    explicit Event(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] int64_t get_last_streamer_in_timestamp() const;
    [[nodiscard]] int get_source_id_trigger() const;

    // template<typename HandlerType>
    // void dispatchTo(HandlerType& handler) {
    //     static_cast<HandlerType&>(handler).handle(*this);
    // }


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

    // template<typename HandlerType>
    // void dispatchTo(HandlerType& handler) {
    //     static_cast<HandlerType&>(handler).handle(*this);
    // }

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

    // template<typename HandlerType>
    // void dispatchTo(HandlerType& handler) {
    //     static_cast<HandlerType&>(handler).handle(*this);
    // }

    protected:
    // int64_t last_order_gateway_in_timestamp;
    // int64_t last_capture_server_in_timestamp;

    MarketTimeStamp market_timestamp;

};



class OrderBookSnapshotEvent : public MarketEvent {
    public:
    OrderBookSnapshotEvent();
    ~OrderBookSnapshotEvent() override =default;
    OrderBookSnapshotEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const OrderBookSnapshotData& order_book_snapshot_data);

    [[nodiscard]] OrderBookSnapshotData get_order_book_snapshot_data() const;

    // template<typename HandlerType>
    // void dispatchTo(HandlerType& handler) {
    //     static_cast<HandlerType&>(handler).handle(*this);
    // }

    protected:
    // map<string,vector<double>> data;
    OrderBookSnapshotData order_book_snapshot_data;



};

class TradeEvent : public MarketEvent {
    public:
    TradeEvent();
    ~TradeEvent() override =default;
    TradeEvent(const MarketTimeStamp& market_time_stamp, const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const int& side,const double& trade_price,const double& base_quantity);

    int get_side() const;
    double get_trade_price() const;
    double get_base_quantity() const;

    // template<typename HandlerType>
    // void dispatchTo(HandlerType& handler) {
    //     static_cast<HandlerType&>(handler).handle(*this);
    // }

    protected:
    int side;
    double trade_price;
    double base_quantity;

};


#endif //MARKETEVENT_H
