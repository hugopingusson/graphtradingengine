//
// Created by hugo on 25/03/25.
//

#ifndef MARKETEVENT_H
#define MARKETEVENT_H

#include <cstdint>
#include <map>
#include <vector>


using namespace std;

class Event {
    public:
    Event();
    virtual ~Event() = default;
    explicit Event(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] int64_t get_last_streamer_in_timestamp() const;
    [[nodiscard]] int get_source_id_trigger() const;



    protected:
    int64_t last_streamer_in_timestamp;
    int source_id_trigger;


};

class HeartBeat : public Event {
    public:
    HeartBeat();
    ~HeartBeat() override =default;
    explicit HeartBeat(const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const double& clock_frequency);

    protected:
    double clock_frequency;
};

class MarketEvent : public Event {
    public:
    MarketEvent();
    ~MarketEvent() override =default;
    MarketEvent(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger);

    [[nodiscard]] int64_t get_last_order_gateway_in_timestamp() const;
    [[nodiscard]] int64_t get_last_capture_server_in_timestamp() const;

    protected:
    int64_t last_order_gateway_in_timestamp;
    int64_t last_capture_server_in_timestamp;
};



class OrderBookSnapshot : public MarketEvent {
    public:
    OrderBookSnapshot();
    ~OrderBookSnapshot() override =default;
    OrderBookSnapshot(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const map<string,vector<double>>& data);

    [[nodiscard]] map<string,vector<double>> get_data() const;

    protected:
    map<string,vector<double>> data;
};

class Trade : public MarketEvent {
    public:
    Trade();
    ~Trade() override =default;
    Trade(const int64_t& last_order_gateway_in_timestamp,const int64_t& last_capture_server_in_timestamp,const int64_t& last_streamer_in_timestamp,const int& source_id_trigger,const int& side,const double& trade_price,const double& base_quantity);

    int get_side() const;
    double get_trade_price() const;
    double get_base_quantity() const;

    protected:
    int side;
    double trade_price;
    double base_quantity;

};


#endif //MARKETEVENT_H
