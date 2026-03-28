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

inline constexpr std::size_t kBookLevels = 10;

enum class Action : std::int32_t { ADD=0, CANCEL=1, MODIFY=2, TRADE=3 };
enum class Side   : std::int32_t { BID=0, ASK=1, NEUTRAL=2 };

struct MarketTimeStamp {
    int64_t capture_server_in_timestamp;
    int64_t order_gateway_in_timestamp;
    int64_t data_gateway_out_timestamp;
};


// Snapshot de carnet L2 borné (compatible legacy)
struct SnapshotData{
    double bid_price[kBookLevels];
    double bid_size[kBookLevels];
    double ask_price[kBookLevels];
    double ask_size[kBookLevels];
};


// Représentation d'un niveau de carnet
struct BookLevel {
    double price{};
    double size{};
    int    count{}; // nombre d'ordres à ce niveau (optionnel)
};

// Ladders triés (bid desc, ask asc)
using AskLadder = std::map<double, BookLevel>;                    // ascendant par prix
using BidLadder = std::map<double, BookLevel, std::greater<double>>; // descendant par prix

// Helpers inline pour hydrater un snapshot borné kBookLevels
inline void ladder_to_snapshot(const BidLadder& bids, const AskLadder& asks, SnapshotData& out, std::size_t depth = kBookLevels) {
    // bids (desc)
    std::size_t i = 0;
    for (auto it = bids.begin(); it != bids.end() && i < depth; ++it, ++i) {
        out.bid_price[i] = it->second.price;
        out.bid_size[i]  = it->second.size;
    }
    for (; i < depth; ++i) {
        out.bid_price[i] = 0.0;
        out.bid_size[i]  = 0.0;
    }

    // asks (asc)
    std::size_t j = 0;
    for (auto it = asks.begin(); it != asks.end() && j < depth; ++it, ++j) {
        out.ask_price[j] = it->second.price;
        out.ask_size[j]  = it->second.size;
    }
    for (; j < depth; ++j) {
        out.ask_price[j] = 0.0;
        out.ask_size[j]  = 0.0;
    }
}

inline void snapshot_to_ladder(const SnapshotData& snap, BidLadder& bids, AskLadder& asks, std::size_t depth = kBookLevels) {
    bids.clear();
    asks.clear();

    // bids: niveau 0..depth-1, prix/size > 0
    for (std::size_t i = 0; i < depth && i < kBookLevels; ++i) {
        double p = snap.bid_price[i];
        double s = snap.bid_size[i];
        if (p > 0.0 && s > 0.0) {
            bids[p] = BookLevel{p, s, 0};
        }
    }

    // asks: niveau 0..depth-1, prix/size > 0
    for (std::size_t i = 0; i < depth && i < kBookLevels; ++i) {
        double p = snap.ask_price[i];
        double s = snap.ask_size[i];
        if (p > 0.0 && s > 0.0) {
            asks[p] = BookLevel{p, s, 0};
        }
    }
}

inline void clear_snapshot(SnapshotData& out) {
    for (std::size_t i = 0; i < kBookLevels; ++i) {
        out.bid_price[i] = 0.0;
        out.bid_size[i]  = 0.0;
        out.ask_price[i] = 0.0;
        out.ask_size[i]  = 0.0;
    }
}


struct ActionData {
    Side side{};
    Action action{};
    std::int32_t layer{};      // fixe la taille
    double price{};
    double base_quantity{};
};

struct TradeData {
    Side side{};
    Action action{Action::TRADE};
    std::int32_t layer{0};
    double price{};
    double base_quantity{};
};



struct MarketByPriceMessage{
    std::string instrument;
    MarketTimeStamp market_time_stamp;
    SnapshotData order_book_snapshot_data;
    ActionData action_data;
};

struct MarketByOrderMessage {
    std::string instrument;
    MarketTimeStamp market_time_stamp;
    ActionData action_data;
};


#pragma pack(pop)

struct HeapItem {
    MarketTimeStamp row;
    size_t file_id;

    bool operator>(const HeapItem& other) const {
        return row.capture_server_in_timestamp > other.row.capture_server_in_timestamp;
    }
};



#endif //DATASTRUCTURE_H
