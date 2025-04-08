//
// Created by hugo on 05/04/25.
//

#ifndef STREAMER_H
#define STREAMER_H


#include <map>
#include <fmt/format.h>
#include <regex>
#include "../Node/Base/MarketNode.h"
#include "../../Logger/Logger.h"
#include "../Node/Base/HeartBeat.h"

using namespace std;
using namespace fmt;


class Streamer {
    public:
    virtual ~Streamer() = default;
    Streamer();

    virtual string get_name()=0;



};




class MarketStreamer : public Streamer {
    public:
    virtual ~MarketStreamer() = default;
    MarketStreamer();
    MarketStreamer(const string &instrument, const string &exchange,const int& trade_source_node_id,const int& order_book_source_node_id);
    MarketStreamer(const string &instrument, const string &exchange);

    string get_instrument();
    string get_exchange();

    void set_order_book_source_node_id(const int& order_book_source_node_id);
    void set_trade_source_node_id(const int& trade_source_node_id);

    int get_order_book_source_node_id();
    int get_trade_source_node_id();

    protected:
    string exchange;
    string instrument;
    int order_book_source_node_id;
    int trade_source_node_id;

};


class HeartBeatStreamer : public Streamer {
    public:
    ~HeartBeatStreamer() override = default;
    HeartBeatStreamer();
    explicit HeartBeatStreamer(const double& frequency);

    void set_heartbeat_source_node_id(const int& heartbeat_source_node_id);
    string get_name() override;
    double get_frequency();
    int get_target_source_node_id();

    protected:
    double frequency;
    int heartbeat_source_node_id;
};




class CmeStreamer : public MarketStreamer {
    public:
    CmeStreamer();
    ~CmeStreamer() override = default;

    string get_name() override;

    CmeStreamer(const string &instrument, const string &exchange);
    CmeStreamer(const string& instrument,const string& exchange,const int& trade_source_node_id,const int& order_book_source_node_id);


};

class StreamerContainer {
    public:
    StreamerContainer();
    StreamerContainer(Logger* logger);
    ~StreamerContainer();


    void add_cme_streamer(Market* market);
    void register_market_source(Market* market);
    void register_heartbeat_source(HeartBeat* heart_beat);

    void register_source(SourceNode* source_node);

    protected:
    Logger* logger;
    vector<MarketStreamer*> market_streamers;
    vector<HeartBeatStreamer*> heartbeat_streamers;

};

#endif //STREAMER_H
