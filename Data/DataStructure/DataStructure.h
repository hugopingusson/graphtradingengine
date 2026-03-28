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


struct OrderBookData{
    double bid_price[kBookLevels];
    double bid_size[kBookLevels];
    double ask_price[kBookLevels];
    double ask_size[kBookLevels];

};


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


struct OrderBookSnapshot{
    MarketTimeStamp market_time_stamp;
    OrderBookData order_book_snapshot_data;
};



struct MarketByPriceMessage{
    std::string instrument;
    MarketTimeStamp market_time_stamp;
    OrderBookData order_book_snapshot_data;
    ActionData action_data;
};

struct MarketByOrderMessage {
    std::string instrument;
    MarketTimeStamp market_time_stamp;
    ActionData action_data;
};

// Minimal snapshot type used by backtest streamer (no instrument stored, only data)
struct MarketByPriceSnapshot {
    MarketTimeStamp market_time_stamp;
    OrderBookData order_book_snapshot_data;
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
