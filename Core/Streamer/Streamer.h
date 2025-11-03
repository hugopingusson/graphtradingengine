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

class MarketStreamer {
public:
    MarketStreamer();
    virtual ~MarketStreamer() = default;
    MarketStreamer(const string &instrument, const string &exchange);

    string get_instrument();
    string get_exchange();


protected:
    string exchange;
    string instrument;

};

class OrderBookStreamer : virtual public MarketStreamer {
    public:
    ~OrderBookStreamer() override = default;
    OrderBookStreamer();
    // OrderBookStreamer(const string &instrument, const string &exchange);

    void set_order_book_source_node_id(const int& order_book_source_node_id);
    int get_order_book_source_node_id();

    protected:
    int order_book_source_node_id;
};

class TradeStreamer : virtual public MarketStreamer {
public:
    ~TradeStreamer() override = default;
    TradeStreamer();
    // TradeStreamer(const string &instrument, const string &exchange);

    void set_trade_source_node_id(const int& trade_source_node_id);
    int get_trade_source_node_id();

protected:
    int trade_source_node_id;
};

class BacktestStreamer: public Streamer{

public:
    BacktestStreamer();
    ~BacktestStreamer() override = default;
    void set_id(const size_t& id);
    virtual bool advance() = 0;
    virtual bool is_good() const = 0;

    virtual HeapItem get_current_heap_item() =0;

protected:
    size_t id;


};

class DatabaseBacktestStreamer: public BacktestStreamer, virtual public OrderBookStreamer, virtual public TradeStreamer {

public:
    DatabaseBacktestStreamer();
    ~DatabaseBacktestStreamer() override = default;
    DatabaseBacktestStreamer(const string &instrument, const string &exchange);
    string route_streamer(const std::string& date);
    string get_name() override;

    bool advance() override;
    bool is_good() const override;
    HeapItem get_current_heap_item() override;
    MarketByPriceSnapshot get_current_market_by_price_snapshot();
    MarketTimeStamp get_current_market_timestamp();

protected:
    std::ifstream file;
    MarketByPriceSnapshot current_market_by_price_snapshot;

};

class HeartBeatBackTestStreamer : public BacktestStreamer {
public:
    ~HeartBeatBackTestStreamer() override = default;
    HeartBeatBackTestStreamer();
    explicit HeartBeatBackTestStreamer(const double& frequency);

    void set_heartbeat_source_node_id(const int& heartbeat_source_node_id);
    string get_name() override;
    double get_frequency();
    int get_target_source_node_id();
    bool advance() override;
    [[nodiscard]] bool is_good() const override;
    HeapItem get_current_heap_item() override;

protected:
    double frequency;
    int heartbeat_source_node_id;
    MarketTimeStamp current_market_timestamp;
};


class BackTestStreamerContainer {
    public:
    BackTestStreamerContainer();
    BackTestStreamerContainer(Logger* logger);
    ~BackTestStreamerContainer();

    map<size_t,BacktestStreamer*> get_streamers();

    void register_source(SourceNode *source_node);
    void register_market_orderbook_source(MarketOrderBook* market);
    void register_market_trade_source(MarketTrade* market);
    void register_heartbeat_source(HeartBeat* heart_beat);

    void route_all_streamers(const string& date);

    // template <typename Derived>
    // void register_source(SourceNode<Derived>* source_node);

    protected:
    size_t max_id;
    Logger* logger;
    map<size_t,BacktestStreamer*> streamers;

    // vector<HeartBeatStreamer*> heartbeat_streamers;

};




// class FullMarketPictureStreamer :virtual public OrderBookStreamer, virtual public TradeStreamer {
//     public:
//     virtual ~FullMarketPictureStreamer() = default;
//     FullMarketPictureStreamer();
// };

#endif //STREAMER_H
