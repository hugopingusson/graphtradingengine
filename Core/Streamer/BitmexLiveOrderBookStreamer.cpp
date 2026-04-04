//
// Created by hugo on 03/04/26.
//

#include "BitmexLiveOrderBookStreamer.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <sstream>
#include <stdexcept>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fmt/core.h>
#include <openssl/ssl.h>

namespace {
using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
}

BitmexLiveOrderBookStreamer::BitmexLiveOrderBookStreamer(const std::string& instrument, size_t ring_capacity)
    : MarketStreamer(instrument, "bitmex"),
      LiveUpdateDeltaOrderBookStreamer(
        fmt::format("BitmexLiveOrderBookStreamer(instrument={})", instrument),
        instrument,
        "bitmex",
        ring_capacity
    ),
      websocket_host("www.bitmex.com"),
      websocket_port("443"),
      websocket_target("/realtime"),
      levels_by_id() {}

void BitmexLiveOrderBookStreamer::run_loop() {
    while (!this->is_stop_requested()) {
        this->levels_by_id.clear();
        this->reset_bootstrap();
        this->connect_and_stream();
        if (!this->is_stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
}

std::string BitmexLiveOrderBookStreamer::subscription_payload() const {
    return fmt::format(
        R"({{"op":"subscribe","args":["orderBookL2_25:{}"]}})",
        this->get_instrument()
    );
}

bool BitmexLiveOrderBookStreamer::connect_and_stream() {
    try {
        boost::asio::io_context ioc;
        ssl::context ssl_ctx(ssl::context::tls_client);
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_verify_mode(ssl::verify_peer);

        tcp::resolver resolver(ioc);
        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ssl_ctx);

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), this->websocket_host.c_str())) {
            throw std::runtime_error("Failed to set TLS SNI hostname for BitMEX websocket");
        }

        auto const results = resolver.resolve(this->websocket_host, this->websocket_port);
        beast::get_lowest_layer(ws).connect(results);
        ws.next_layer().handshake(ssl::stream_base::client);

        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "GraphTradingEngine/bitmex");
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
        std::cerr << "BitmexLiveOrderBookStreamer connection/read error: " << e.what() << std::endl;
        return false;
    }
}


bool BitmexLiveOrderBookStreamer::handle_raw_message(const std::string& raw_message) {
    boost::property_tree::ptree message;
    std::istringstream input(raw_message);
    try {
        boost::property_tree::read_json(input, message);
    } catch (const std::exception&) {
        return false;
    }

    const auto table_opt = message.get_optional<std::string>("table");
    const auto action_opt = message.get_optional<std::string>("action");
    const auto data_opt = message.get_child_optional("data");
    if (!table_opt || !action_opt || !data_opt) {
        return false;
    }
    if (*table_opt != "orderBookL2" && *table_opt != "orderBookL2_25") {
        return false;
    }

    const int64_t streamer_ts = now_ns();
    const int64_t message_exchange_ts = get_exchange_timestamp(message, nullptr, streamer_ts);

    const std::string& action = *action_opt;
    if (action == "partial") {
        return this->handle_partial(*data_opt, message_exchange_ts);
    }
    if (action == "insert" || action == "update" || action == "delete") {
        return this->handle_deltas(action, *data_opt, message_exchange_ts);
    }

    return false;
}

bool BitmexLiveOrderBookStreamer::handle_partial(const boost::property_tree::ptree& data_array, const int64_t& default_exchange_timestamp) {
    this->levels_by_id.clear();

    int64_t reference_exchange_ts = default_exchange_timestamp;
    bool has_any_level = false;

    for (const auto& row_node : data_array) {
        const auto& row = row_node.second;

        const auto symbol = row.get<std::string>("symbol", "");
        if (!symbol.empty() && symbol != this->get_instrument()) {
            continue;
        }

        const auto id_opt = row.get_optional<int64_t>("id");
        const auto side_opt = row.get_optional<std::string>("side");
        const auto price_opt = row.get_optional<double>("price");
        const auto size_opt = row.get_optional<double>("size");
        if (!id_opt || !side_opt || !price_opt || !size_opt) {
            continue;
        }

        const Side side = parse_side(*side_opt);
        if (side == Side::NEUTRAL) {
            continue;
        }

        Update cached_update{};
        cached_update.level = BookLevel{*price_opt, *size_opt, (*price_opt) * (*size_opt), 0};
        cached_update.side = side;
        cached_update.action = Action::ADD;
        this->levels_by_id[*id_opt] = cached_update;
        has_any_level = true;
        reference_exchange_ts = get_exchange_timestamp(boost::property_tree::ptree{}, &row, reference_exchange_ts);
    }

    if (!has_any_level) {
        return false;
    }

    SnapshotData snapshot{};
    this->rebuild_snapshot(snapshot);

    const int64_t streamer_ts = now_ns();
    MarketTimeStamp market_ts{};
    market_ts.order_gateway_in_timestamp = reference_exchange_ts;
    market_ts.capture_server_in_timestamp = reference_exchange_ts;
    market_ts.data_gateway_out_timestamp = streamer_ts;

    MarketByPriceMessage message{};
    message.market_time_stamp = market_ts;
    message.order_book_snapshot_data = snapshot;

    this->mark_bootstrapped();
    return this->emit_mbp_event(
        market_ts,
        reference_exchange_ts,
        streamer_ts,
        this->get_order_book_source_node_id(),
        message
    );
}

bool BitmexLiveOrderBookStreamer::handle_deltas(const std::string& action, const boost::property_tree::ptree& data_array, const int64_t& default_exchange_timestamp) {
    if (!this->is_bootstrapped()) {
        return false;
    }

    bool pushed_any = false;

    for (const auto& row_node : data_array) {
        const auto& row = row_node.second;

        const auto symbol = row.get<std::string>("symbol", "");
        if (!symbol.empty() && symbol != this->get_instrument()) {
            continue;
        }

        const auto id_opt = row.get_optional<int64_t>("id");
        if (!id_opt) {
            continue;
        }
        const int64_t level_id = *id_opt;

        Update current{};
        auto current_it = this->levels_by_id.find(level_id);
        const bool exists = current_it != this->levels_by_id.end();
        if (exists) {
            current = current_it->second;
        }

        if (action == "delete" && !exists) {
            continue;
        }
        if ((action == "update" || action == "insert") && !exists && !row.get_optional<std::string>("side")) {
            continue;
        }

        const auto side_opt = row.get_optional<std::string>("side");
        const auto price_opt = row.get_optional<double>("price");
        const auto size_opt = row.get_optional<double>("size");

        const Side side = side_opt ? parse_side(*side_opt) : current.side;
        if (side == Side::NEUTRAL) {
            continue;
        }

        const double price = price_opt ? *price_opt : current.level.price;
        const double size = size_opt ? *size_opt : current.level.size;

        Action update_action = Action::MODIFY;
        if (action == "insert") {
            update_action = Action::ADD;
            Update cached_update{};
            cached_update.level = BookLevel{price, size, price * size, 0};
            cached_update.side = side;
            cached_update.action = update_action;
            this->levels_by_id[level_id] = cached_update;
        } else if (action == "update") {
            update_action = Action::MODIFY;
            Update cached_update{};
            cached_update.level = BookLevel{price, size, price * size, 0};
            cached_update.side = side;
            cached_update.action = update_action;
            this->levels_by_id[level_id] = cached_update;
        } else if (action == "delete") {
            update_action = Action::CANCEL;
            this->levels_by_id.erase(level_id);
        } else {
            continue;
        }

        const int64_t streamer_ts = now_ns();
        const int64_t exchange_ts = get_exchange_timestamp(boost::property_tree::ptree{}, &row, default_exchange_timestamp);

        MarketTimeStamp market_ts{};
        market_ts.order_gateway_in_timestamp = exchange_ts;
        market_ts.capture_server_in_timestamp = exchange_ts;
        market_ts.data_gateway_out_timestamp = streamer_ts;

        Update update{};
        update.market_time_stamp = market_ts;
        update.level = BookLevel{price, size, price * size, 0};
        update.action = update_action;
        update.side = side;

        MarketUpdateMessage message{};
        message.market_time_stamp = market_ts;
        message.update = update;

        pushed_any |= this->emit_update_event(
            market_ts,
            exchange_ts,
            streamer_ts,
            this->get_order_book_source_node_id(),
            message
        );
    }

    return pushed_any;
}

void BitmexLiveOrderBookStreamer::rebuild_snapshot(SnapshotData& out_snapshot) const {
    BidLadder bids;
    AskLadder asks;

    for (const auto& kv : this->levels_by_id) {
        const Update& level_update = kv.second;
        const BookLevel& level = level_update.level;
        if (level.size <= 0.0 || !std::isfinite(level.price) || !std::isfinite(level.size)) {
            continue;
        }

        if (level_update.side == Side::BID) {
            bids[level.price] = BookLevel{level.price, level.size, level.price * level.size, 0};
        } else if (level_update.side == Side::ASK) {
            asks[level.price] = BookLevel{level.price, level.size, level.price * level.size, 0};
        }
    }

    clear_snapshot(out_snapshot);
    ladder_to_snapshot(bids, asks, out_snapshot, kBookLevels);
}

Side BitmexLiveOrderBookStreamer::parse_side(const std::string& side) {
    if (side == "Buy" || side == "BID" || side == "bid") {
        return Side::BID;
    }
    if (side == "Sell" || side == "ASK" || side == "ask") {
        return Side::ASK;
    }
    return Side::NEUTRAL;
}

int64_t BitmexLiveOrderBookStreamer::now_ns() {
    return Timestamp::now_unix(Resolution::nanoseconds);
}

int64_t BitmexLiveOrderBookStreamer::parse_iso8601_to_ns(const std::string& timestamp) {
    if (timestamp.size() < 20) {
        throw std::runtime_error(fmt::format("Invalid ISO8601 timestamp '{}'", timestamp));
    }

    // Expected: YYYY-MM-DDTHH:MM:SS[.fraction][Z]
    const int year = std::stoi(timestamp.substr(0, 4));
    const int month = std::stoi(timestamp.substr(5, 2));
    const int day = std::stoi(timestamp.substr(8, 2));
    const int hour = std::stoi(timestamp.substr(11, 2));
    const int minute = std::stoi(timestamp.substr(14, 2));
    const int second = std::stoi(timestamp.substr(17, 2));

    size_t pos = 19;
    int64_t nanos = 0;
    if (pos < timestamp.size() && timestamp[pos] == '.') {
        ++pos;
        int digits = 0;
        while (pos < timestamp.size() && std::isdigit(static_cast<unsigned char>(timestamp[pos]))) {
            if (digits < 9) {
                nanos = nanos * 10 + static_cast<int64_t>(timestamp[pos] - '0');
                ++digits;
            }
            ++pos;
        }
        while (digits < 9) {
            nanos *= 10;
            ++digits;
        }
    }

    // Convert civil date -> epoch seconds (UTC).
    auto days_from_civil = [](int y, unsigned m, unsigned d) -> int64_t {
        y -= m <= 2;
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned m_adj = m > 2 ? m - 3U : m + 9U;
        const unsigned doy = (153U * m_adj + 2U) / 5U + d - 1U;
        const unsigned doe = yoe * 365U + yoe / 4U - yoe / 100U + doy;
        return static_cast<int64_t>(era) * 146097LL + static_cast<int64_t>(doe) - 719468LL;
    };

    const int64_t days = days_from_civil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
    const int64_t day_seconds = static_cast<int64_t>(hour) * 3600LL
        + static_cast<int64_t>(minute) * 60LL
        + static_cast<int64_t>(second);
    return (days * 86400LL + day_seconds) * 1000000000LL + nanos;
}

int64_t BitmexLiveOrderBookStreamer::get_exchange_timestamp(const boost::property_tree::ptree& message,
                                                            const boost::property_tree::ptree* row,
                                                            const int64_t& fallback_timestamp) {
    if (row) {
        const auto row_ts_opt = row->get_optional<std::string>("timestamp");
        if (row_ts_opt) {
            try {
                return parse_iso8601_to_ns(*row_ts_opt);
            } catch (const std::exception&) {
            }
        }
    }

    const auto message_ts_opt = message.get_optional<std::string>("timestamp");
    if (message_ts_opt) {
        try {
            return parse_iso8601_to_ns(*message_ts_opt);
        } catch (const std::exception&) {
        }
    }

    return fallback_timestamp;
}
