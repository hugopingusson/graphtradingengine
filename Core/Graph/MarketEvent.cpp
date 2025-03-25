//
// Created by hugo on 25/03/25.
//

#include "MarketEvent.h"

#include "../Node/Market.h"


MarketEvent::MarketEvent():last_exchange_timestamp(int64_t()),last_reception_timestamp(int64_t()) {};
MarketEvent::MarketEvent(const int64_t &last_reception_timestamp, const int64_t &last_exchange_timestamp):last_exchange_timestamp(last_exchange_timestamp),last_reception_timestamp(last_reception_timestamp) {};


SnapshotEvent::SnapshotEvent():MarketEvent(),data(map<string,vector<double>>()) {}
