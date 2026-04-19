//
// Created by hugo on 04/04/26.
//

#ifndef BINANCELIVEORDERBOOKSTREAMER_H
#define BINANCELIVEORDERBOOKSTREAMER_H

#include <cstdint>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "LiveStreamer.h"

class BinanceLiveOrderBookStreamer : public LiveSnapshotOrderBookStreamer {
public:
    explicit BinanceLiveOrderBookStreamer(const std::string& instrument,
                                          size_t ring_capacity = 1u << 16,
                                          size_t max_update_batch_size = 64);
    ~BinanceLiveOrderBookStreamer() override = default;

protected:
    void run_loop() override;

private:
    bool connect_and_stream();
    bool handle_raw_message(std::string_view raw_message);
    bool handle_depth_message(const nlohmann::json& payload);

    std::string stream_target() const;

    static bool to_double(const nlohmann::json& value, double& out);
    static int64_t to_ns_timestamp(const nlohmann::json& value, const int64_t& fallback_ns);
    static int64_t now_ns();

    std::string websocket_host;
    std::string websocket_port;
};

#endif //BINANCELIVEORDERBOOKSTREAMER_H
