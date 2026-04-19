//
// Created by hugo on 04/04/26.
//

#ifndef OKXLIVEORDERBOOKSTREAMER_H
#define OKXLIVEORDERBOOKSTREAMER_H

#include <cstdint>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "LiveStreamer.h"

class OkxLiveOrderBookStreamer : public LiveSnapshotOrderBookStreamer {
public:
    explicit OkxLiveOrderBookStreamer(const std::string& instrument,
                                      size_t ring_capacity = 1u << 16,
                                      size_t max_update_batch_size = 64);
    ~OkxLiveOrderBookStreamer() override = default;

protected:
    void run_loop() override;

private:
    bool connect_and_stream();
    std::string subscription_payload() const;
    bool handle_raw_message(std::string_view raw_message);
    bool handle_order_book_data(const nlohmann::json& data_message);

    static bool to_double(const nlohmann::json& value, double& out);
    static int64_t to_ns_timestamp(const nlohmann::json& value, const int64_t& fallback_ns);
    static int64_t now_ns();

    std::string websocket_host;
    std::string websocket_port;
    std::string websocket_target;
};

#endif //OKXLIVEORDERBOOKSTREAMER_H
