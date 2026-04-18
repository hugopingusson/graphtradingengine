//
// Created by hugo on 04/04/26.
//

#include "DeribitLiveOrderBookStreamer.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <fmt/core.h>
#include <openssl/ssl.h>

namespace {
using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using Json = nlohmann::json;
}

DeribitLiveOrderBookStreamer::DeribitLiveOrderBookStreamer(const std::string& instrument, size_t ring_capacity)
    : MarketStreamer(instrument, "deribit"),
      LiveUpdateDeltaOrderBookStreamer(
          fmt::format("DeribitLiveOrderBookStreamer(instrument={})", instrument),
          instrument,
          "deribit",
          ring_capacity
      ),
      websocket_host("www.deribit.com"),
      websocket_port("443"),
      websocket_target("/ws/api/v2"),
      bid_ladder(),
      ask_ladder() {}

void DeribitLiveOrderBookStreamer::run_loop() {
    while (!this->is_stop_requested()) {
        this->bid_ladder.clear();
        this->ask_ladder.clear();
        this->reset_bootstrap();
        this->connect_and_stream();
        if (!this->is_stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
}

bool DeribitLiveOrderBookStreamer::connect_and_stream() {
    try {
        boost::asio::io_context ioc;
        ssl::context ssl_ctx(ssl::context::tls_client);
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_verify_mode(ssl::verify_peer);

        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ssl_ctx);
        tcp::resolver resolver(ioc);

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), this->websocket_host.c_str())) {
            throw std::runtime_error("Failed to set TLS SNI hostname for Deribit websocket");
        }

        auto const results = resolver.resolve(this->websocket_host, this->websocket_port);
        beast::get_lowest_layer(ws).connect(results);
        ws.next_layer().handshake(ssl::stream_base::client);

        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "GraphTradingEngine/deribit");
        }));
        ws.handshake(this->websocket_host, this->websocket_target);

        const std::string subscribe = this->subscription_payload();
        ws.write(boost::asio::buffer(subscribe));

        while (!this->is_stop_requested()) {
            beast::flat_buffer buffer;
            ws.read(buffer);
            const std::string raw_message = beast::buffers_to_string(buffer.cdata());
            this->handle_raw_message(raw_message);
        }

        boost::system::error_code close_ec;
        ws.close(websocket::close_code::normal, close_ec);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "DeribitLiveOrderBookStreamer connection/read error: " << e.what() << std::endl;
        return false;
    }
}

std::string DeribitLiveOrderBookStreamer::subscription_payload() const {
    return fmt::format(
        R"({{"jsonrpc":"2.0","id":1,"method":"public/subscribe","params":{{"channels":["book.{}.none.10.100ms"]}}}})",
        this->get_instrument()
    );
}

bool DeribitLiveOrderBookStreamer::handle_raw_message(const std::string& raw_message) {
    Json message = Json::parse(raw_message, nullptr, false);
    if (message.is_discarded() || !message.is_object()) {
        return false;
    }

    const auto method_it = message.find("method");
    if (method_it == message.end() || !method_it->is_string()) {
        return false;
    }
    if (method_it->get<std::string>() != "subscription") {
        return false;
    }

    const auto params_it = message.find("params");
    if (params_it == message.end() || !params_it->is_object()) {
        return false;
    }

    const auto channel_it = params_it->find("channel");
    const auto data_it = params_it->find("data");
    if (channel_it == params_it->end() || data_it == params_it->end()) {
        return false;
    }
    if (!channel_it->is_string() || !data_it->is_object()) {
        return false;
    }

    const std::string channel = channel_it->get<std::string>();
    if (channel.rfind("book.", 0) != 0) {
        return false;
    }

    return this->handle_subscription_data(*data_it);
}

bool DeribitLiveOrderBookStreamer::handle_subscription_data(const Json& data_message) {
    const auto bids_it = data_message.find("bids");
    const auto asks_it = data_message.find("asks");
    if (bids_it == data_message.end() || asks_it == data_message.end()) {
        return false;
    }
    if (!bids_it->is_array() || !asks_it->is_array()) {
        return false;
    }

    std::vector<ParsedLevel> bids;
    std::vector<ParsedLevel> asks;
    bids.reserve(bids_it->size());
    asks.reserve(asks_it->size());

    for (const auto& level_json : *bids_it) {
        ParsedLevel level;
        if (parse_level(level_json, level)) {
            bids.push_back(level);
        }
    }
    for (const auto& level_json : *asks_it) {
        ParsedLevel level;
        if (parse_level(level_json, level)) {
            asks.push_back(level);
        }
    }

    const int64_t streamer_ts = now_ns();
    const auto ts_it = data_message.find("timestamp");
    const int64_t exchange_ts = to_ns_timestamp(
        ts_it != data_message.end() ? *ts_it : Json(),
        streamer_ts
    );

    const auto type_it = data_message.find("type");
    if (type_it != data_message.end() && type_it->is_string() && type_it->get<std::string>() == "snapshot") {
        return this->handle_snapshot(bids, asks, exchange_ts);
    }

    return this->handle_change(bids, asks, exchange_ts);
}

bool DeribitLiveOrderBookStreamer::handle_snapshot(const std::vector<ParsedLevel>& bids, const std::vector<ParsedLevel>& asks, const int64_t& exchange_ts) {
    this->bid_ladder.clear();
    this->ask_ladder.clear();

    for (const auto& level : bids) {
        this->apply_level_to_ladder(Side::BID, level);
    }
    for (const auto& level : asks) {
        this->apply_level_to_ladder(Side::ASK, level);
    }

    if (this->bid_ladder.empty() || this->ask_ladder.empty()) {
        return false;
    }

    SnapshotData snapshot{};
    this->rebuild_snapshot(snapshot);

    const int64_t streamer_ts = now_ns();
    MarketTimeStamp market_ts{};
    market_ts.order_gateway_in_timestamp = exchange_ts;
    market_ts.data_gateway_out_timestamp = streamer_ts;

    SnapshotMessage message{};
    message.market_time_stamp = market_ts;
    message.order_book_snapshot_data = snapshot;

    this->mark_bootstrapped();
    return this->emit_mbp_event(
        streamer_ts,
        this->get_order_book_source_node_id(),
        message
    );
}

bool DeribitLiveOrderBookStreamer::handle_change(const std::vector<ParsedLevel>& bids, const std::vector<ParsedLevel>& asks, const int64_t& exchange_ts) {
    if (!this->is_bootstrapped()) {
        return false;
    }

    const int64_t streamer_ts = now_ns();
    bool pushed_any = false;

    for (const auto& level : bids) {
        this->apply_level_to_ladder(Side::BID, level);
        pushed_any |= this->emit_update(Side::BID, level, exchange_ts, streamer_ts);
    }
    for (const auto& level : asks) {
        this->apply_level_to_ladder(Side::ASK, level);
        pushed_any |= this->emit_update(Side::ASK, level, exchange_ts, streamer_ts);
    }

    return pushed_any;
}

bool DeribitLiveOrderBookStreamer::apply_level_to_ladder(const Side side, const ParsedLevel& level) {
    if (side == Side::NEUTRAL || !std::isfinite(level.price) || level.price <= 0.0) {
        return false;
    }

    if (side == Side::BID) {
        if (level.action == Action::CANCEL || level.size <= 0.0) {
            this->bid_ladder.erase(level.price);
            return true;
        }
        this->bid_ladder[level.price] = BookLevel{level.price, level.size, level.price * level.size, 0};
        return true;
    }

    if (level.action == Action::CANCEL || level.size <= 0.0) {
        this->ask_ladder.erase(level.price);
        return true;
    }
    this->ask_ladder[level.price] = BookLevel{level.price, level.size, level.price * level.size, 0};
    return true;
}

bool DeribitLiveOrderBookStreamer::emit_update(const Side side, const ParsedLevel& level, const int64_t& exchange_ts, const int64_t& streamer_ts) {
    if (side == Side::NEUTRAL || !std::isfinite(level.price) || level.price <= 0.0) {
        return false;
    }

    const double normalized_size = (level.action == Action::CANCEL || level.size <= 0.0) ? 0.0 : level.size;

    MarketTimeStamp market_ts{};
    market_ts.order_gateway_in_timestamp = exchange_ts;
    market_ts.data_gateway_out_timestamp = streamer_ts;

    Update update{};
    update.market_time_stamp = market_ts;
    update.level = BookLevel{level.price, normalized_size, level.price * normalized_size, 0};
    update.action = level.action;
    update.side = side;

    UpdateMessage message{};
    message.market_time_stamp = market_ts;
    message.update = update;

    return this->emit_update_event(
        streamer_ts,
        this->get_order_book_source_node_id(),
        message
    );
}

bool DeribitLiveOrderBookStreamer::parse_level(const Json& level_json, ParsedLevel& out_level) {
    if (!level_json.is_array()) {
        return false;
    }

    if (level_json.size() >= 3 && level_json[0].is_string()) {
        const Action action = parse_action(level_json[0]);
        double price = 0.0;
        double size = 0.0;
        if (!to_double(level_json[1], price) || !to_double(level_json[2], size)) {
            return false;
        }

        out_level.action = action;
        out_level.price = price;
        out_level.size = size;
        return true;
    }

    if (level_json.size() >= 2) {
        double price = 0.0;
        double size = 0.0;
        if (!to_double(level_json[0], price) || !to_double(level_json[1], size)) {
            return false;
        }

        out_level.action = (size <= 0.0) ? Action::CANCEL : Action::MODIFY;
        out_level.price = price;
        out_level.size = size;
        return true;
    }

    return false;
}

bool DeribitLiveOrderBookStreamer::to_double(const Json& value, double& out) {
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

int64_t DeribitLiveOrderBookStreamer::to_ns_timestamp(const Json& value, const int64_t& fallback_ns) {
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

Action DeribitLiveOrderBookStreamer::parse_action(const Json& action_value) {
    if (!action_value.is_string()) {
        return Action::MODIFY;
    }

    const std::string action = action_value.get<std::string>();
    if (action == "new") {
        return Action::ADD;
    }
    if (action == "delete") {
        return Action::CANCEL;
    }
    if (action == "change") {
        return Action::MODIFY;
    }
    return Action::MODIFY;
}

int64_t DeribitLiveOrderBookStreamer::now_ns() {
    return Timestamp::now_unix(Resolution::nanoseconds);
}

void DeribitLiveOrderBookStreamer::rebuild_snapshot(SnapshotData& out_snapshot) const {
    clear_snapshot(out_snapshot);
    ladder_to_snapshot(this->bid_ladder, this->ask_ladder, out_snapshot, kBookLevels);
}
