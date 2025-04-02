//
// Created by hugo on 25/03/25.
//

#include "Event.h"

#include "../Node/Market.h"


Event::Event():last_reception_timestamp(int64_t()){};
Event::Event(const int64_t &last_reception_timestamp):last_reception_timestamp(last_reception_timestamp) {};

MarketEvent::MarketEvent():Event(),last_exchange_timestamp(int64_t()),last_adapter_timestamp(int64_t()) {};
MarketEvent::MarketEvent(const int64_t& last_exchange_timestamp,const int64_t& last_adapter_timestamp,const int64_t& last_reception_timestamp):Event(last_reception_timestamp),last_exchange_timestamp(last_exchange_timestamp),last_adapter_timestamp(last_adapter_timestamp) {};



MarketSnapshot::MarketSnapshot():MarketEvent() {}
MarketSnapshot::MarketSnapshot(const int64_t &last_exchange_timestamp, const int64_t &last_adapter_timestamp, const int64_t &last_reception_timestamp) {
    this->last_exchange_timestamp = last_exchange_timestamp;
    this->last_adapter_timestamp = last_adapter_timestamp;
    this->last_reception_timestamp = last_reception_timestamp;
    this->data=map<string,vector<double>>();
}




MarketSnapshot::MarketSnapshot(const int64_t &last_exchange_timestamp, const int64_t &last_adapter_timestamp, const int64_t &last_reception_timestamp, const map<string, vector<double> > &data, const string &action, const bool &linked) {
    this->last_exchange_timestamp = last_exchange_timestamp;
    this->last_adapter_timestamp = last_adapter_timestamp;
    this->last_reception_timestamp = last_reception_timestamp;
    this->data = data;
    this->action = action;
    this->linked = linked;
}

