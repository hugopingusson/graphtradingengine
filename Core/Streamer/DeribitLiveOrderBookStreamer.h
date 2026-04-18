//
// Created by hugo on 04/04/26.
//

#ifndef DERIBITLIVEORDERBOOKSTREAMER_H
#define DERIBITLIVEORDERBOOKSTREAMER_H

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "LiveStreamer.h"

class DeribitLiveOrderBookStreamer : public LiveUpdateDeltaOrderBookStreamer {
public:
    explicit DeribitLiveOrderBookStreamer(const std::string& instrument, size_t ring_capacity = 1u << 16);
    ~DeribitLiveOrderBookStreamer() override = default;

protected:
    void run_loop() override;

private:
    struct ParsedLevel {
        Action action{Action::MODIFY};
        double price{0.0};
        double size{0.0};
    };

    bool connect_and_stream();
    std::string subscription_payload() const;
    bool handle_raw_message(const std::string& raw_message);
    bool handle_subscription_data(const nlohmann::json& data_message);

    bool handle_snapshot(const std::vector<ParsedLevel>& bids, const std::vector<ParsedLevel>& asks, const int64_t& exchange_ts);
    bool handle_change(const std::vector<ParsedLevel>& bids, const std::vector<ParsedLevel>& asks, const int64_t& exchange_ts);
    bool apply_level_to_ladder(Side side, const ParsedLevel& level);
    bool emit_update(Side side, const ParsedLevel& level, const int64_t& exchange_ts, const int64_t& streamer_ts);

    static bool parse_level(const nlohmann::json& level_json, ParsedLevel& out_level);
    static bool to_double(const nlohmann::json& value, double& out);
    static int64_t to_ns_timestamp(const nlohmann::json& value, const int64_t& fallback_ns);
    static Action parse_action(const nlohmann::json& action_value);
    static int64_t now_ns();

    void rebuild_snapshot(SnapshotData& out_snapshot) const;

    std::string websocket_host;
    std::string websocket_port;
    std::string websocket_target;

    BidLadder bid_ladder;
    AskLadder ask_ladder;
};

#endif //DERIBITLIVEORDERBOOKSTREAMER_H
