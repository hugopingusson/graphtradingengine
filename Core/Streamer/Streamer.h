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

    string get_instrument() const;
    string get_exchange() const;
    void set_order_book_source_node_id(const int& order_book_source_node_id);
    int get_order_book_source_node_id() const;

protected:
    string exchange;
    string instrument;
    int order_book_source_node_id;
};

class BacktestStreamer : public Streamer {
public:
    BacktestStreamer();
    ~BacktestStreamer() override = default;
    void set_id(const size_t& id);

    virtual bool advance() = 0;
    virtual bool is_good() const = 0;
    virtual HeapItem get_current_heap_item() = 0;
    virtual void set_and_route(const Timestamp& start, const Timestamp& end)=0;

protected:
    size_t id;
};

class DatabaseWMBPBacktestStreamer : public BacktestStreamer,
                                     virtual public MarketStreamer {
public:
    DatabaseWMBPBacktestStreamer();
    ~DatabaseWMBPBacktestStreamer() override = default;
    DatabaseWMBPBacktestStreamer(const string& instrument, const string& exchange);

    void set_and_route(const Timestamp& start, const Timestamp& end) override;
    string get_name() override;
    // std::ifstream get_file();
    bool advance() override;
    HeapItem get_current_heap_item() override;
    bool is_good() const override;

    WideMarketByPriceMessage get_current_message() const;

protected:
    std::ifstream file;
    WideMarketByPriceMessage current_message;
};

class HeartBeatBackTestStreamer : public BacktestStreamer {
public:
    ~HeartBeatBackTestStreamer() override = default;
    HeartBeatBackTestStreamer();
    explicit HeartBeatBackTestStreamer(const double& frequency);

    void set_heartbeat_source_node_id(const int& heartbeat_source_node_id);
    void set_and_route(const Timestamp &start, const Timestamp &end) override;
    string get_name() override;
    double get_frequency();
    bool advance() override;
    [[nodiscard]] bool is_good() const override;
    HeapItem get_current_heap_item() override;

protected:
    double frequency;
    int heartbeat_source_node_id;
    int64_t capture_in_server_timestamp;
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
    void register_heartbeat_source(HeartBeat* heart_beat);

    void route_and_set_streamers(const Timestamp& start,const Timestamp& end);

protected:
    size_t max_id;
    Logger* logger;
    map<size_t, BacktestStreamer*> streamers;
};

#endif //STREAMER_H
