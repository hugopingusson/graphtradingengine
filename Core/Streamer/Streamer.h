//
// Created by hugo on 05/04/25.
//

#ifndef STREAMER_H
#define STREAMER_H


#include <map>
#include <fmt/format.h>

#include "../Node/Base/MarketNode.h"

using namespace std;
using namespace fmt;




class Streamer {
    public:
    virtual ~Streamer() = default;
    Streamer();

    protected:
    MarketOrderBook* order_book_source_node;
    MarketTrade* trade_book_source_node;

};


class CmeStreamer : public Streamer {
    public:
    CmeStreamer();
    ~CmeStreamer() override = default;
    CmeStreamer(const string& exchange,const string& instrument);

    protected:
    string exchange;
    string instrument;

};

class StreamerContainer {
    public:
    StreamerContainer();
    ~StreamerContainer();
    void add_streamer(const string& exchange,const string& pair);


    protected:
    int max_id;
    map<int,Streamer*> streamers;

};

#endif //STREAMER_H
