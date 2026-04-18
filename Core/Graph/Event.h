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
    Event(const int64_t& reception_timestamp,
          const int& source_id_trigger,
          const Location& location = Location::UNKNOWN,
          const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] int64_t get_reception_timestamp() const;
    [[nodiscard]] Location get_location() const;
    [[nodiscard]] Listener get_listener() const;
    [[nodiscard]] int get_source_id_trigger() const;

    // Double dispatch entry point
    virtual void dispatchTo(MarketOrderBook& target) = 0;


    protected:
    int64_t reception_timestamp;
    int source_id_trigger;
    Location location;
    Listener listener;


};

class HeartBeatEvent : public Event {
    public:
    HeartBeatEvent();
    ~HeartBeatEvent() override =default;
    HeartBeatEvent(const int64_t& reception_timestamp,
                   const int& source_id_trigger,
                   const double& clock_frequency,
                   const Location& location = Location::UNKNOWN,
                   const Listener& listener = Listener::UNKNOWN);

    double get_frequency() const;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    double frequency;
};



class MarketEvent : public Event {
    public:
    MarketEvent();
    ~MarketEvent() override =default;
    MarketEvent(const string& instrument,
                const int64_t& reception_timestamp,
                const int& source_id_trigger,
                const Location& location = Location::UNKNOWN,
                const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] const string& get_instrument() const;
    [[nodiscard]] virtual MarketTimeStamp get_last_market_timestamp() const = 0;

    void dispatchTo(MarketOrderBook& target) override;


    protected:

    string instrument;

};



class MarketByPriceEvent : public MarketEvent {
    public:
    MarketByPriceEvent();
    ~MarketByPriceEvent() override =default;
    MarketByPriceEvent(const string& instrument,
                       const int64_t& reception_timestamp,
                       const int& source_id_trigger,
                       const MarketByPriceMessage& message,
                       const Location& location = Location::UNKNOWN,
                       const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] MarketByPriceMessage get_message() const;
    [[nodiscard]] SnapshotData get_snapshot_data() const;
    [[nodiscard]] Order get_order() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const override;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    MarketByPriceMessage message;
};


class SnapshotEvent : public MarketEvent {
    public:
    SnapshotEvent();
    ~SnapshotEvent() override =default;
    SnapshotEvent(const string& instrument,
                  const int64_t& reception_timestamp,
                  const int& source_id_trigger,
                  const SnapshotMessage& mbp_message,
                  const Location& location = Location::UNKNOWN,
                  const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] SnapshotMessage get_message() const;
    [[nodiscard]] SnapshotData get_snapshot_data() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const override;

    void dispatchTo(MarketOrderBook& target) override;

    protected:
    SnapshotMessage mbp_message;

};

class OrderEvent : public MarketEvent {
public:
    OrderEvent();
    ~OrderEvent() override =default;
    OrderEvent(const string& instrument,
               const int64_t& reception_timestamp,
               const int& source_id_trigger,
               const OrderMessage& mbo_message,
               const Location& location = Location::UNKNOWN,
               const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] OrderMessage get_message() const;
    [[nodiscard]] Order get_order() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const override;

    void dispatchTo(MarketOrderBook& target) override;

protected:
    OrderMessage mbo_message;

};

class UpdateEvent : public MarketEvent {
public:
    UpdateEvent();
    ~UpdateEvent() override =default;
    UpdateEvent(const string& instrument,
                const int64_t& reception_timestamp,
                const int& source_id_trigger,
                const UpdateMessage& mbo_message,
                const Location& location = Location::UNKNOWN,
                const Listener& listener = Listener::UNKNOWN);

    [[nodiscard]] UpdateMessage get_message() const;
    [[nodiscard]] Update get_update() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const override;


    void dispatchTo(MarketOrderBook& target) override;

protected:
    UpdateMessage update_message;

};


class TradeEvent : public MarketEvent {
    public:
    TradeEvent();
    ~TradeEvent() override =default;
    TradeEvent(const string& instrument,
               const MarketTimeStamp& market_time_stamp,
               const int64_t& reception_timestamp,
               const int& source_id_trigger,
               const Side& side,
               const double& trade_price,
               const double& base_quantity,
               const Location& location = Location::UNKNOWN,
               const Listener& listener = Listener::UNKNOWN);

    Side get_side() const;
    double get_trade_price() const;
    double get_base_quantity() const;
    [[nodiscard]] MarketTimeStamp get_last_market_timestamp() const override;


    protected:
    Side side;
    double trade_price;
    double base_quantity;
    MarketTimeStamp trade_market_timestamp;

};


#endif //MARKETEVENT_H
