//
// Created by hugo on 05/04/25.
//

#include "Streamer.h"




Streamer::Streamer() {}

CmeStreamer::CmeStreamer():Streamer() {}
CmeStreamer::CmeStreamer(const string &exchange, const string &instrument):exchange(exchange),instrument(instrument) {}

StreamerContainer::StreamerContainer():max_id(0) {}

StreamerContainer::~StreamerContainer() {
    for (int i = 0; i < streamers.size(); i++) {
        delete streamers[i];
    }
}

void StreamerContainer::add_streamer(const string& exchange,const string& instrument) {
    if (exchange == "cme") {
        max_id+=1;
        this->streamers[max_id]=new CmeStreamer(exchange,instrument);
    }

    throw std::logic_error(format("No streamer available for exchange={} pair={}", exchange, instrument));

}
