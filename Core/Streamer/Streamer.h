//
// Created by hugo on 05/04/25.
//

#ifndef STREAMER_H
#define STREAMER_H

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

#include <fmt/format.h>

#include "../Node/Base/HeartBeat.h"
#include "../Node/Base/MarketNode.h"
#include "../../Data/DataStructure/DataStructure.h"
#include "../../Logger/Logger.h"

using namespace std;
using namespace fmt;

class Streamer {
public:
    virtual ~Streamer() = default;
    Streamer();
    virtual string get_name() = 0;
};

class MarketStreamer {
public:
    MarketStreamer();
    virtual ~MarketStreamer() = default;
    MarketStreamer(const string& instrument, const string& exchange);

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

    void set_order_book_source_node_id(const int& order_book_source_node_id);
    int get_order_book_source_node_id();

protected:
    int order_book_source_node_id;
};

class TradeStreamer : virtual public MarketStreamer {
public:
    ~TradeStreamer() override = default;
    TradeStreamer();

    void set_trade_source_node_id(const int& trade_source_node_id);
    int get_trade_source_node_id();

protected:
    int trade_source_node_id;
};

class BacktestStreamer : public Streamer {
public:
    BacktestStreamer();
    ~BacktestStreamer() override = default;
    void set_id(const size_t& id);

    virtual bool advance() = 0;
    virtual bool is_good() const = 0;
    virtual HeapItem get_current_heap_item() = 0;

    // Event remains the unique contract between streamer and graph.
    virtual Event* materialize_event() = 0;
    virtual int get_target_source_node_id() const = 0;

protected:
    size_t id;
};

class DatabaseBacktestStreamer : public BacktestStreamer,
                                 virtual public MarketStreamer,
                                 virtual public OrderBookStreamer,
                                 virtual public TradeStreamer {
public:
    DatabaseBacktestStreamer();
    ~DatabaseBacktestStreamer() override = default;
    DatabaseBacktestStreamer(const string& instrument, const string& exchange);

    string route_streamer(const string& date);
    string get_name() override;

    bool advance() override;
    bool is_good() const override;
    HeapItem get_current_heap_item() override;
    Event* materialize_event() override;
    int get_target_source_node_id() const override;

    MarketByPriceMessage get_current_market_by_price_message();
    // Compatibility alias kept to avoid touching callers outside Streamer files.
    MarketByPriceMessage get_current_market_by_price_snapshot();
    MarketTimeStamp get_current_market_timestamp();
    Action get_current_action() const;
    Side get_current_side() const;
    double get_current_price() const;
    double get_current_base_quantity() const;

protected:
    std::ifstream file;
    MarketByPriceMessage current_market_by_price_message;
    Action current_action;
    Side current_side;
    double current_price;
    double current_base_quantity;
};

class HeartBeatBackTestStreamer : public BacktestStreamer {
public:
    ~HeartBeatBackTestStreamer() override = default;
    HeartBeatBackTestStreamer();
    explicit HeartBeatBackTestStreamer(const double& frequency);

    void set_heartbeat_source_node_id(const int& heartbeat_source_node_id);
    string get_name() override;
    double get_frequency();
    int get_target_source_node_id() const override;
    bool advance() override;
    [[nodiscard]] bool is_good() const override;
    HeapItem get_current_heap_item() override;
    Event* materialize_event() override;

protected:
    double frequency;
    int heartbeat_source_node_id;
    bool emitted_once;
    MarketTimeStamp current_market_timestamp;
};

class BackTestStreamerContainer {
public:
    BackTestStreamerContainer();
    BackTestStreamerContainer(Logger* logger);
    ~BackTestStreamerContainer();

    map<size_t, BacktestStreamer*>& get_streamers();
    const map<size_t, BacktestStreamer*>& get_streamers() const;

    void register_source(Producer* source_node);
    void register_market_orderbook_source(MarketOrderBook* market);
    void register_market_trade_source(const string& instrument, const string& exchange, const int& node_id);
    void register_heartbeat_source(HeartBeat* heart_beat);

    void route_all_streamers(const string& date);

protected:
    size_t max_id;
    Logger* logger;
    map<size_t, BacktestStreamer*> streamers;
};

#endif //STREAMER_H
