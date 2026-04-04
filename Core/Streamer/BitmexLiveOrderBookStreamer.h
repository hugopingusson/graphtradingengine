//
// Created by hugo on 03/04/26.
//

#ifndef BITMEXLIVEORDERBOOKSTREAMER_H
#define BITMEXLIVEORDERBOOKSTREAMER_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>

#include "LiveStreamer.h"

class BitmexLiveOrderBookStreamer : public LiveUpdateDeltaOrderBookStreamer {
public:
    explicit BitmexLiveOrderBookStreamer(const std::string& instrument, size_t ring_capacity = 1u << 16);
    ~BitmexLiveOrderBookStreamer() override = default;

protected:
    void run_loop() override;

private:
    bool connect_and_stream();
    std::string subscription_payload() const;

    bool handle_raw_message(const std::string& raw_message);
    bool handle_partial(const boost::property_tree::ptree& data_array, const int64_t& default_exchange_timestamp);
    bool handle_deltas(const std::string& action, const boost::property_tree::ptree& data_array, const int64_t& default_exchange_timestamp);

    static Side parse_side(const std::string& side);
    static int64_t now_ns();
    static int64_t parse_iso8601_to_ns(const std::string& timestamp);
    static int64_t get_exchange_timestamp(const boost::property_tree::ptree& message,
                                          const boost::property_tree::ptree* row,
                                          const int64_t& fallback_timestamp);

    void rebuild_snapshot(SnapshotData& out_snapshot) const;

    std::string websocket_host;
    std::string websocket_port;
    std::string websocket_target;

    std::unordered_map<int64_t, Update> levels_by_id;
};

#endif //BITMEXLIVEORDERBOOKSTREAMER_H
