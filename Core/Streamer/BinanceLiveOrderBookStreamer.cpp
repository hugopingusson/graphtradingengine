//
// Created by hugo on 04/04/26.
//

#include "BinanceLiveOrderBookStreamer.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <thread>

#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>

namespace {
using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using Json = nlohmann::json;
}

BinanceLiveOrderBookStreamer::BinanceLiveOrderBookStreamer(const std::string& instrument,
                                                           size_t ring_capacity,
                                                           size_t max_update_batch_size)
    : MarketStreamer(instrument, "binance"),
      LiveSnapshotOrderBookStreamer(
          fmt::format("BinanceLiveOrderBookStreamer(instrument={})", instrument),
          instrument,
          "binance",
          ring_capacity,
          max_update_batch_size
      ),
      websocket_host("stream.binance.com"),
      websocket_port("9443") {}

void BinanceLiveOrderBookStreamer::run_loop() {
    while (!this->is_stop_requested()) {
        this->connect_and_stream();
        if (!this->is_stop_requested() && !this->is_desynced()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
}

bool BinanceLiveOrderBookStreamer::connect_and_stream() {
    try {
        boost::asio::io_context ioc;
        ssl::context ssl_ctx(ssl::context::tls_client);
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_verify_mode(ssl::verify_peer);

        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ssl_ctx);
        tcp::resolver resolver(ioc);

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), this->websocket_host.c_str())) {
            throw std::runtime_error("Failed to set TLS SNI hostname for Binance websocket");
        }

        auto const results = resolver.resolve(this->websocket_host, this->websocket_port);
        beast::get_lowest_layer(ws).connect(results);
        ws.next_layer().handshake(ssl::stream_base::client);

        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "GraphTradingEngine/binance");
        }));
        ws.handshake(this->websocket_host, this->stream_target());

        beast::flat_buffer buffer;
        while (!this->is_stop_requested()) {
            if (this->consume_reconnect_request()) {
                break;
            }
            buffer.consume(buffer.size());
            ws.read(buffer);
            const auto data = buffer.cdata();
            if (data.size() > 0) {
                const auto* raw_ptr = static_cast<const char*>(data.data());
                const std::string_view raw_message(raw_ptr, data.size());
                this->handle_raw_message(raw_message);
            }
            if (this->consume_reconnect_request()) {
                break;
            }
        }

        boost::system::error_code close_ec;
        ws.close(websocket::close_code::normal, close_ec);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "BinanceLiveOrderBookStreamer connection/read error: " << e.what() << std::endl;
        return false;
    }
}

bool BinanceLiveOrderBookStreamer::handle_raw_message(std::string_view raw_message) {
    Json message = Json::parse(raw_message.begin(), raw_message.end(), nullptr, false);
    if (message.is_discarded() || !message.is_object()) {
        return false;
    }

    const Json* payload = &message;
    const auto data_it = message.find("data");
    if (data_it != message.end() && data_it->is_object()) {
        payload = &(*data_it);
    }

    const bool has_diff_depth = payload->find("b") != payload->end() && payload->find("a") != payload->end();
    const bool has_partial_depth = payload->find("bids") != payload->end() && payload->find("asks") != payload->end();
    if (!has_diff_depth && !has_partial_depth) {
        return false;
    }

    return this->handle_depth_message(*payload);
}

bool BinanceLiveOrderBookStreamer::handle_depth_message(const Json& payload) {
    const auto bids_diff_it = payload.find("b");
    const auto asks_diff_it = payload.find("a");
    const auto bids_partial_it = payload.find("bids");
    const auto asks_partial_it = payload.find("asks");

    const Json* bids_ptr = nullptr;
    const Json* asks_ptr = nullptr;
    if (bids_diff_it != payload.end() && asks_diff_it != payload.end()) {
        bids_ptr = &(*bids_diff_it);
        asks_ptr = &(*asks_diff_it);
    } else if (bids_partial_it != payload.end() && asks_partial_it != payload.end()) {
        bids_ptr = &(*bids_partial_it);
        asks_ptr = &(*asks_partial_it);
    } else {
        return false;
    }

    if (!bids_ptr->is_array() || !asks_ptr->is_array()) {
        return false;
    }

    BidLadder bids;
    AskLadder asks;

    for (const auto& level : *bids_ptr) {
        if (!level.is_array() || level.size() < 2) {
            continue;
        }
        double price = 0.0;
        double size = 0.0;
        if (!to_double(level[0], price) || !to_double(level[1], size)) {
            continue;
        }
        if (!std::isfinite(price) || !std::isfinite(size) || price <= 0.0 || size <= 0.0) {
            continue;
        }
        bids[price] = BookLevel{price, size, price * size, 0};
    }

    for (const auto& level : *asks_ptr) {
        if (!level.is_array() || level.size() < 2) {
            continue;
        }
        double price = 0.0;
        double size = 0.0;
        if (!to_double(level[0], price) || !to_double(level[1], size)) {
            continue;
        }
        if (!std::isfinite(price) || !std::isfinite(size) || price <= 0.0 || size <= 0.0) {
            continue;
        }
        asks[price] = BookLevel{price, size, price * size, 0};
    }

    if (bids.empty() || asks.empty()) {
        return false;
    }

    const int64_t streamer_ts = now_ns();
    const auto event_time_it = payload.find("E");
    const int64_t exchange_ts = to_ns_timestamp(
        event_time_it != payload.end() ? *event_time_it : Json(),
        streamer_ts
    );

    SnapshotData snapshot{};
    clear_snapshot(snapshot);
    ladder_to_snapshot(bids, asks, snapshot, kBookLevels);

    MarketTimeStamp market_ts{};
    market_ts.order_gateway_in_timestamp = exchange_ts;
    market_ts.data_gateway_out_timestamp = streamer_ts;

    SnapshotMessage message{};
    message.market_time_stamp = market_ts;
    message.order_book_snapshot_data = snapshot;

    return this->emit_mbp_event(
        streamer_ts,
        this->get_order_book_source_node_id(),
        message
    );
}

std::string BinanceLiveOrderBookStreamer::stream_target() const {
    std::string stream = this->get_instrument();
    std::transform(stream.begin(), stream.end(), stream.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return fmt::format("/ws/{}@depth20@100ms", stream);
}

bool BinanceLiveOrderBookStreamer::to_double(const Json& value, double& out) {
    if (value.is_number_float()) {
        out = value.get<double>();
        return true;
    }
    if (value.is_number_integer()) {
        out = static_cast<double>(value.get<int64_t>());
        return true;
    }
    if (value.is_number_unsigned()) {
        out = static_cast<double>(value.get<uint64_t>());
        return true;
    }
    if (value.is_string()) {
        const std::string& s = value.get_ref<const std::string&>();
        char* end_ptr = nullptr;
        out = std::strtod(s.c_str(), &end_ptr);
        return end_ptr != s.c_str() && end_ptr && *end_ptr == '\0';
    }
    return false;
}

int64_t BinanceLiveOrderBookStreamer::to_ns_timestamp(const Json& value, const int64_t& fallback_ns) {
    int64_t raw = 0;
    if (value.is_number_integer()) {
        raw = value.get<int64_t>();
    } else if (value.is_number_unsigned()) {
        const auto v = value.get<uint64_t>();
        if (v > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return fallback_ns;
        }
        raw = static_cast<int64_t>(v);
    } else if (value.is_string()) {
        const std::string& s = value.get_ref<const std::string&>();
        if (s.empty()) {
            return fallback_ns;
        }
        try {
            raw = std::stoll(s);
        } catch (...) {
            return fallback_ns;
        }
    } else {
        return fallback_ns;
    }

    if (raw <= 0) {
        return fallback_ns;
    }
    if (raw < 1'000'000'000'000LL) {
        return raw * 1'000'000'000LL;
    }
    if (raw < 1'000'000'000'000'000LL) {
        return raw * 1'000'000LL;
    }
    if (raw < 1'000'000'000'000'000'000LL) {
        return raw * 1'000LL;
    }
    return raw;
}

int64_t BinanceLiveOrderBookStreamer::now_ns() {
    return Timestamp::now_unix(Resolution::nanoseconds);
}
