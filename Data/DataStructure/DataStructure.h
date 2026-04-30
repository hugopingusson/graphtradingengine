//
// Created by hugo on 13/04/25.
//

#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <algorithm>

#include "../../Helper/Configuration.h"

#pragma pack(push, 1)

inline constexpr std::size_t kBookLevels = 25;

struct BookLevel {
    double price{};
    double size{};
    double amount{};
    int count{};
};

enum class Action : std::int32_t { ADD=0, CANCEL=1, MODIFY=2, TRADE=3 };
// enum class Update : std::int32_t { INSERT=0, DELETE=1, UPDATE=2 };
enum class Side   : std::int32_t { BID=0, ASK=1, NEUTRAL=2 };
enum class Location : std::uint8_t { UNKNOWN=0, NYK=1, CHI=2, LON=3, SIN=4, PAR=5 };
enum class Listener : std::uint8_t { UNKNOWN=0, DATABENTO=1, CRYPTOLAKE=2, PRODUCTION=3 };

struct MarketTimeStamp {
    int64_t order_gateway_in_timestamp;
    int64_t data_gateway_out_timestamp;
};


// Fixed-size L2 order book snapshot (legacy-compatible)
struct SnapshotData{
    double bid_price[kBookLevels];
    double bid_size[kBookLevels];
    std::int32_t bid_count[kBookLevels];
    double ask_price[kBookLevels];
    double ask_size[kBookLevels];
    std::int32_t ask_count[kBookLevels];
};

// Sorted ladders (bid desc, ask asc)
using AskLadder = std::map<double, BookLevel>;                    // ascending by price
using BidLadder = std::map<double, BookLevel, std::greater<double>>; // descending by price

// Inline helpers to populate a bounded kBookLevels snapshot
inline void ladder_to_snapshot(const BidLadder& bids, const AskLadder& asks, SnapshotData& out, std::size_t depth = kBookLevels) {
    // bids (desc)
    std::size_t i = 0;
    for (auto it = bids.begin(); it != bids.end() && i < depth; ++it, ++i) {
        out.bid_price[i] = it->second.price;
        out.bid_size[i]  = it->second.size;
        out.bid_count[i] = static_cast<std::int32_t>(it->second.count);
    }
    for (; i < depth; ++i) {
        out.bid_price[i] = 0.0;
        out.bid_size[i]  = 0.0;
        out.bid_count[i] = 0;
    }

    // asks (asc)
    std::size_t j = 0;
    for (auto it = asks.begin(); it != asks.end() && j < depth; ++it, ++j) {
        out.ask_price[j] = it->second.price;
        out.ask_size[j]  = it->second.size;
        out.ask_count[j] = static_cast<std::int32_t>(it->second.count);
    }
    for (; j < depth; ++j) {
        out.ask_price[j] = 0.0;
        out.ask_size[j]  = 0.0;
        out.ask_count[j] = 0;
    }
}

inline void snapshot_to_ladder(const SnapshotData& snap, BidLadder& bids, AskLadder& asks, std::size_t depth = kBookLevels) {
    bids.clear();
    asks.clear();

    // bids: levels 0..depth-1, price/size > 0
    for (std::size_t i = 0; i < depth && i < kBookLevels; ++i) {
        double p = snap.bid_price[i];
        double s = snap.bid_size[i];
        if (p > 0.0 && s > 0.0) {
            bids[p] = BookLevel{p, s, p*s, static_cast<int>(snap.bid_count[i])};
        }
    }

    // asks: levels 0..depth-1, price/size > 0
    for (std::size_t i = 0; i < depth && i < kBookLevels; ++i) {
        double p = snap.ask_price[i];
        double s = snap.ask_size[i];
        if (p > 0.0 && s > 0.0) {
            asks[p] = BookLevel{p, s, p*s, static_cast<int>(snap.ask_count[i])};
        }
    }
}

inline void clear_snapshot(SnapshotData& out) {
    for (std::size_t i = 0; i < kBookLevels; ++i) {
        out.bid_price[i] = 0.0;
        out.bid_size[i]  = 0.0;
        out.bid_count[i] = 0;
        out.ask_price[i] = 0.0;
        out.ask_size[i]  = 0.0;
        out.ask_count[i] = 0;
    }
}


struct Order {
    Side side{};
    Action action{};
    std::int32_t layer{};      // fixed-width integer
    double price{};
    double size{};
};

struct Update {
    MarketTimeStamp market_time_stamp;
    BookLevel level;
    Action action;
    Side side;
};

struct OrderMessage {
    MarketTimeStamp market_time_stamp;
    Order order;
};

struct UpdateMessage {
    MarketTimeStamp market_time_stamp;
    Update update;
};

struct SnapshotMessage{
    MarketTimeStamp market_time_stamp;
    SnapshotData order_book_snapshot_data;
};

struct MarketByPriceMessage{
    MarketTimeStamp market_time_stamp;
    SnapshotData order_book_snapshot_data;
    Order order;
};

// On-disk POD payload for a MarketByPriceEvent.
struct MarketByPriceEventPod {
    int64_t reception_timestamp;
    Location location;
    Listener listener;
    MarketByPriceMessage message;
};


#pragma pack(pop)

struct HeapItem {
    int64_t row;
    size_t file_id;

    bool operator>(const HeapItem& other) const {
        return row > other.row;
    }
};



#endif //DATASTRUCTURE_H
