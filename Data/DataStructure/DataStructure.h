//
// Created by hugo on 13/04/25.
//

#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>

#pragma pack(push, 1)

enum class Action : std::int32_t { ADD=0, CANCEL=1, MODIFY=2, TRADE=3 };
enum class Side   : std::int32_t { BID=0, ASK=1, NEUTRAL=2 };

struct MarketTimeStamp {
    int64_t capture_server_in_timestamp;
    int64_t order_gateway_in_timestamp;
    int64_t data_gateway_out_timestamp;
};


// struct MarketDataPoint {
//     MarketTimeStamp market_timestamp;
// };

struct OrderBookSnapshotData{
    double bid_price[10];
    double bid_size[10];
    double ask_price[10];
    double ask_size[10];

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
    OrderBookSnapshotData order_book_snapshot_data;
};



struct MarketByPriceSnapshot{
    // OrderBookSnapshot order_book_snapshot;
    MarketTimeStamp market_time_stamp;
    OrderBookSnapshotData order_book_snapshot_data;
    ActionData action_data;
};



// struct Order:MarketDataPoint{
//     Action action;
// };

#pragma pack(pop)

struct HeapItem {
    MarketTimeStamp row;
    size_t file_id;

    bool operator>(const HeapItem& other) const {
        return row.capture_server_in_timestamp > other.row.capture_server_in_timestamp;
    }
};



// struct StreamCursor {
//     std::ifstream file;
//     OrderBookSnapshot current;
//     size_t file_id;
//
//     StreamCursor(const std::string& path, size_t id) : file_id(id) {
//         file.open(path, std::ios::binary);
//         advance();
//     }
//
//     bool advance() {
//         file.read(reinterpret_cast<char*>(&current), sizeof(OrderBookSnapshot));
//         return file.gcount() == sizeof(OrderBookSnapshot);
//     }
//
//     bool is_good() const {
//         return file.good();
//     }
// };




#endif //DATASTRUCTURE_H
