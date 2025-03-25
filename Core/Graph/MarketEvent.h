//
// Created by hugo on 25/03/25.
//

#ifndef MARKETEVENT_H
#define MARKETEVENT_H

#include <cstdint>
#include <map>
#include <vector>

using namespace std;

class MarketEvent {
    public:
    virtual ~MarketEvent() = default;
    MarketEvent();
    MarketEvent(const int64_t& last_reception_timestamp,const int64_t& last_exchange_timestamp);

    protected:
    int64_t last_reception_timestamp;
    int64_t last_exchange_timestamp;

};

class SnapshotEvent : MarketEvent {
    public:
    SnapshotEvent();
    SnapshotEvent(const int64_t& last_reception_timestamp,const int64_t& last_exchange_timestamp,const map<string,vector<double>>& data);

    protected:
    map<string,vector<double>> data;
};



#endif //MARKETEVENT_H
