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
    Event(const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] int64_t get_capture_server_in_timestamp() const;
    [[nodiscard]] int64_t get_streamer_in_timestamp() const;
    [[nodiscard]] int get_source_id_trigger() const;

    // Double dispatch entry point
    virtual void dispatchTo(MarketOrderBook& target) = 0;


    protected:
    int64_t capture_server_in_timestamp;
    int64_t streamer_in_timestamp;
    int source_id_trigger;


};

class HeartBeatEvent : public Event {
    public:
    HeartBeatEvent();
    ~HeartBeatEvent() override =default;
    HeartBeatEvent(const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency);

    double get_frequency() const;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    double frequency;
};



class MarketEvent : public Event {
    public:
    MarketEvent();
    ~MarketEvent() override =default;
    MarketEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] const string& get_instrument() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const;

    void dispatchTo(MarketOrderBook& target) override;


    protected:

    MarketTimeStamp market_timestamp;
    string instrument;

};



class MBPEvent : public MarketEvent {
    public:
    MBPEvent();
    ~MBPEvent() override =default;
    MBPEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketByPriceMessage& mbp_message);

    [[nodiscard]] MarketByPriceMessage get_message() const;
    [[nodiscard]] SnapshotData get_snapshot_data() const;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    MarketByPriceMessage mbp_message;

};

class MBOEvent : public MarketEvent {
public:
    MBOEvent();
    ~MBOEvent() override =default;
    MBOEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketByOrderMessage& mbo_message);

    [[nodiscard]] MarketByOrderMessage get_message() const;
    [[nodiscard]] Order get_order() const;

    void dispatchTo(MarketOrderBook& target) override;

protected:
    MarketByOrderMessage mbo_message;

};

class UpdateEvent : public MarketEvent {
public:
    UpdateEvent();
    ~UpdateEvent() override =default;
    UpdateEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const MarketUpdateMessage& mbo_message);

    [[nodiscard]] MarketUpdateMessage get_message() const;
    [[nodiscard]] Update get_update() const;


    void dispatchTo(MarketOrderBook& target) override;

protected:
    MarketUpdateMessage update_message;

};


class TradeEvent : public MarketEvent {
    public:
    TradeEvent();
    ~TradeEvent() override =default;
    TradeEvent(const string& instrument,const MarketTimeStamp& market_time_stamp,const int64_t& capture_server_in_timestamp,const int64_t& streamer_in_timestamp,const int& source_id_trigger,const Side& side,const double& trade_price,const double& base_quantity);

    Side get_side() const;
    double get_trade_price() const;
    double get_base_quantity() const;


    protected:
    Side side;
    double trade_price;
    double base_quantity;

};


#endif //MARKETEVENT_H
