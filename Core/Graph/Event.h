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
    virtual ~Event() = default;
    Event();
    Event(const int64_t& last_reception_timestamp);

    protected:
    int64_t last_reception_timestamp;
    int source_id_trigger;


};

// class HeartBeat : public Event

class MarketEvent : public Event {
    public:
    virtual ~MarketEvent() = default;
    MarketEvent();
    MarketEvent(const int64_t& last_exchange_timestamp,const int64_t& last_adapter_timestamp,const int64_t& last_reception_timestamp);

    protected:
    int64_t last_exchange_timestamp;
    int64_t last_adapter_timestamp;
};

// class ActionMarketEvent : public MarketEvent


class MarketSnapshot : public MarketEvent {
    public:
    MarketSnapshot();
    MarketSnapshot(const int64_t& last_exchange_timestamp,const int64_t& last_adapter_timestamp,const int64_t& last_reception_timestamp);
    MarketSnapshot(const int64_t& last_exchange_timestamp,const int64_t& last_adapter_timestamp,const int64_t& last_reception_timestamp,const map<string,vector<double>>& data,const string& action,const bool& linked);

    protected:
    map<string,vector<double>> data;
    string action;
    bool linked;
};



#endif //MARKETEVENT_H
