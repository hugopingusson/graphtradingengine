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


enum Action {
    ADD=0,
    CANCEL=1,
    MODIFY=2,
    TRADE=3,
};

enum Side {
    BID=0,
    ASK=1,
    NEUTRAL=2
};


struct MarketTimeStamp {
    int64_t capture_server_in_timestamp;
    int64_t order_gateway_in_timestamp;
    int64_t data_gateway_out_timestamp;
};


struct MarketDataPoint {
    MarketTimeStamp market_timestamp;
};

struct OrderBookSnapshotData{
    double bid_price[10];
    double bid_size[10];
    double ask_price[10];
    double ask_size[10];

};


struct ActionData {
    Side side;
    Action action;
    int layer;
    double price;
    double base_quantity;
};

struct TradeData {
    Side side;
    const Action action = TRADE;
    const int layer=0;
    double price;
    double base_quantity;
};

#pragma pack(push, 1)
struct OrderBookSnapshot: MarketDataPoint{
    OrderBookSnapshotData order_book_snapshot_data;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MarketByPriceSnapshot:MarketDataPoint{
    ActionData action_data;
    OrderBookSnapshotData order_book_snapshot_data;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Order:MarketDataPoint{
    Action action;
};
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
