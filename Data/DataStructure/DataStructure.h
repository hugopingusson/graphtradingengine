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

struct MarketTimeStamp {
    int64_t capture_server_in_timestamp;
    int64_t order_gateway_in_timestamp;
    int64_t data_gateway_out_timestamp;
};

struct OrderBookSnapshotData{
    double bid_price[10];
    double bid_size[10];
    double ask_price[10];
    double ask_size[10];

};

#pragma pack(push, 1)
struct OrderBookSnapshot{
    MarketTimeStamp market_timestamp;
    OrderBookSnapshotData order_book_snapshot_data;
};
#pragma pack(pop)

struct StreamCursor {
    std::ifstream file;
    OrderBookSnapshot current;
    size_t file_id;

    StreamCursor(const std::string& path, size_t id) : file_id(id) {
        file.open(path, std::ios::binary);
        advance();
    }

    bool advance() {
        file.read(reinterpret_cast<char*>(&current), sizeof(OrderBookSnapshot));
        return file.gcount() == sizeof(OrderBookSnapshot);
    }

    bool is_good() const {
        return file.good();
    }
};

struct HeapItem {
    OrderBookSnapshot row;
    size_t file_id;

    bool operator>(const HeapItem& other) const {
        return row.market_timestamp.capture_server_in_timestamp > other.row.market_timestamp.capture_server_in_timestamp;
    }
};



#endif //DATASTRUCTURE_H
