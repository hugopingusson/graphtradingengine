//
// Created by hugo on 03/04/26.
//

#ifndef BITMEXLIVEORDERBOOKSTREAMER_H
#define BITMEXLIVEORDERBOOKSTREAMER_H

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LiveStreamer.h"

class BitmexLiveOrderBookStreamer : public LiveUpdateDeltaOrderBookStreamer {
public:
    explicit BitmexLiveOrderBookStreamer(const std::string& instrument,
                                         size_t ring_capacity = 1u << 16,
                                         size_t max_update_batch_size = 64);
    ~BitmexLiveOrderBookStreamer() override = default;

protected:
    void run_loop() override;

private:
    struct ParsedL2Row {
        bool symbol_matches{true};
        bool has_id{false};
        int64_t id{};
        bool has_side{false};
        Side side{Side::NEUTRAL};
        bool has_price{false};
        double price{};
        bool has_size{false};
        double size{};
        bool has_timestamp{false};
        int64_t exchange_timestamp{};
    };

    bool connect_and_stream();
    std::string subscription_payload() const;

    bool handle_raw_message(std::string_view raw_message);
    bool handle_partial(const std::vector<ParsedL2Row>& rows, const int64_t& default_exchange_timestamp);
    bool handle_deltas(const std::string& action, const std::vector<ParsedL2Row>& rows, const int64_t& default_exchange_timestamp);

    static Side parse_side(std::string_view side);
    static int64_t now_ns();
    static int64_t parse_iso8601_to_ns(std::string_view timestamp);

    void rebuild_snapshot(SnapshotData& out_snapshot) const;

    std::string websocket_host;
    std::string websocket_port;
    std::string websocket_target;

    std::unordered_map<int64_t, Update> levels_by_id;
};

#endif //BITMEXLIVEORDERBOOKSTREAMER_H
