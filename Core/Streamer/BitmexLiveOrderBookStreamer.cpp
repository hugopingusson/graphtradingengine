//
// Created by hugo on 03/04/26.
//

#include "BitmexLiveOrderBookStreamer.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <thread>
#include <sstream>
#include <stdexcept>

#include <boost/asio/connect.hpp>
#include <boost/asio/buffer.hpp>
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

inline bool to_int64(const Json& value, int64_t& out) {
    if (value.is_number_integer()) {
        out = value.get<int64_t>();
        return true;
    }
    if (value.is_number_unsigned()) {
        const auto v = value.get<uint64_t>();
        if (v > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return false;
        }
        out = static_cast<int64_t>(v);
        return true;
    }
    return false;
}

inline bool to_double(const Json& value, double& out) {
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
    return false;
}
}

BitmexLiveOrderBookStreamer::BitmexLiveOrderBookStreamer(const std::string& instrument,
                                                         size_t ring_capacity,
                                                         size_t max_update_batch_size)
    : MarketStreamer(instrument, "bitmex"),
      LiveUpdateDeltaOrderBookStreamer(
        fmt::format("BitmexLiveOrderBookStreamer(instrument={})", instrument),
        instrument,
        "bitmex",
        ring_capacity,
        max_update_batch_size
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
        if (!this->is_stop_requested() && !this->is_desynced()) {
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
        std::cerr << "BitmexLiveOrderBookStreamer connection/read error: " << e.what() << std::endl;
        return false;
    }
}


bool BitmexLiveOrderBookStreamer::handle_raw_message(std::string_view raw_message) {
    Json message = Json::parse(raw_message.begin(), raw_message.end(), nullptr, false);
    if (message.is_discarded() || !message.is_object()) {
        return false;
    }

    const auto table_it = message.find("table");
    const auto action_it = message.find("action");
    const auto data_it = message.find("data");
    if (table_it == message.end() || action_it == message.end() || data_it == message.end()) {
        return false;
    }
    if (!table_it->is_string() || !action_it->is_string() || !data_it->is_array()) {
        return false;
    }

    const std::string& table = table_it->get_ref<const std::string&>();
    if (table != "orderBookL2" && table != "orderBookL2_25") {
        return false;
    }

    const int64_t streamer_ts = now_ns();
    int64_t message_exchange_ts = streamer_ts;

    const auto message_ts_it = message.find("timestamp");
    if (message_ts_it != message.end() && message_ts_it->is_string()) {
        const std::string& ts_ref = message_ts_it->get_ref<const std::string&>();
        try {
            message_exchange_ts = parse_iso8601_to_ns(ts_ref);
        } catch (const std::exception&) {
        }
    }

    const std::string& action = action_it->get_ref<const std::string&>();

    const Json& data_array = *data_it;
    std::vector<ParsedL2Row> rows;
    rows.reserve(data_array.size());

    const std::string instrument = this->get_instrument();
    for (const auto& row_value : data_array) {
        if (!row_value.is_object()) {
            continue;
        }
        const Json& row_object = row_value;
        ParsedL2Row row;

        const auto symbol_it = row_object.find("symbol");
        if (symbol_it != row_object.end() && symbol_it->is_string()) {
            const std::string& symbol = symbol_it->get_ref<const std::string&>();
            row.symbol_matches = symbol.empty() || symbol == instrument;
        }

        const auto id_it = row_object.find("id");
        if (id_it != row_object.end()) {
            row.has_id = to_int64(*id_it, row.id);
        }

        const auto side_it = row_object.find("side");
        if (side_it != row_object.end() && side_it->is_string()) {
            const std::string& side_ref = side_it->get_ref<const std::string&>();
            row.side = parse_side(side_ref);
            row.has_side = true;
        }

        const auto price_it = row_object.find("price");
        if (price_it != row_object.end()) {
            row.has_price = to_double(*price_it, row.price);
        }

        const auto size_it = row_object.find("size");
        if (size_it != row_object.end()) {
            row.has_size = to_double(*size_it, row.size);
        }

        const auto ts_it = row_object.find("timestamp");
        if (ts_it != row_object.end() && ts_it->is_string()) {
            const std::string& ts_ref = ts_it->get_ref<const std::string&>();
            try {
                row.exchange_timestamp = parse_iso8601_to_ns(ts_ref);
                row.has_timestamp = true;
            } catch (const std::exception&) {
            }
        }

        rows.push_back(row);
    }

    if (action == "partial") {
        return this->handle_partial(rows, message_exchange_ts);
    }
    if (action == "insert" || action == "update" || action == "delete") {
        return this->handle_deltas(action, rows, message_exchange_ts);
    }

    return false;
}

bool BitmexLiveOrderBookStreamer::handle_partial(const std::vector<ParsedL2Row>& rows, const int64_t& default_exchange_timestamp) {
    this->levels_by_id.clear();

    int64_t reference_exchange_ts = default_exchange_timestamp;
    bool has_any_level = false;

    for (const auto& row : rows) {
        if (!row.symbol_matches) {
            continue;
        }
        if (!row.has_id || !row.has_side || !row.has_price || !row.has_size) {
            continue;
        }
        if (row.side == Side::NEUTRAL) {
            continue;
        }

        Update cached_update{};
        cached_update.level = BookLevel{row.price, row.size, row.price * row.size, 0};
        cached_update.side = row.side;
        cached_update.action = Action::ADD;
        this->levels_by_id[row.id] = cached_update;
        has_any_level = true;
        if (row.has_timestamp) {
            reference_exchange_ts = row.exchange_timestamp;
        }
    }

    if (!has_any_level) {
        return false;
    }

    SnapshotData snapshot{};
    this->rebuild_snapshot(snapshot);

    const int64_t streamer_ts = now_ns();
    MarketTimeStamp market_ts{};
    market_ts.order_gateway_in_timestamp = reference_exchange_ts;
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

bool BitmexLiveOrderBookStreamer::handle_deltas(const std::string& action, const std::vector<ParsedL2Row>& rows, const int64_t& default_exchange_timestamp) {
    if (!this->is_bootstrapped()) {
        return false;
    }

    std::vector<UpdateMessage> batch_messages;
    batch_messages.reserve(rows.size());
    const int64_t payload_streamer_ts = now_ns();

    for (const auto& row : rows) {
        if (!row.symbol_matches) {
            continue;
        }
        if (!row.has_id) {
            continue;
        }
        const int64_t level_id = row.id;

        Update current{};
        auto current_it = this->levels_by_id.find(level_id);
        const bool exists = current_it != this->levels_by_id.end();
        if (exists) {
            current = current_it->second;
        }

        if (action == "delete" && !exists) {
            continue;
        }
        if ((action == "update" || action == "insert") && !exists && !row.has_side) {
            continue;
        }

        const Side side = row.has_side ? row.side : current.side;
        if (side == Side::NEUTRAL) {
            continue;
        }

        const double price = row.has_price ? row.price : current.level.price;
        const double size = row.has_size ? row.size : current.level.size;

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

        const int64_t exchange_ts = row.has_timestamp ? row.exchange_timestamp : default_exchange_timestamp;

        MarketTimeStamp market_ts{};
        market_ts.order_gateway_in_timestamp = exchange_ts;
        market_ts.data_gateway_out_timestamp = payload_streamer_ts;

        Update update{};
        update.market_time_stamp = market_ts;
        update.level = BookLevel{price, size, price * size, 0};
        update.action = update_action;
        update.side = side;

        UpdateMessage message{};
        message.market_time_stamp = market_ts;
        message.update = update;

        batch_messages.push_back(message);
    }

    if (batch_messages.empty()) {
        return false;
    }

    return this->emit_update_batch_event(
        payload_streamer_ts,
        this->get_order_book_source_node_id(),
        batch_messages
    );
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

Side BitmexLiveOrderBookStreamer::parse_side(std::string_view side) {
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

int64_t BitmexLiveOrderBookStreamer::parse_iso8601_to_ns(std::string_view timestamp) {
    if (timestamp.size() < 19) {
        throw std::runtime_error("Invalid ISO8601 timestamp");
    }

    auto digit = [](const char c) -> int {
        if (c < '0' || c > '9') {
            return -1;
        }
        return c - '0';
    };
    auto parse2 = [&](const size_t pos) -> int {
        const int d0 = digit(timestamp[pos]);
        const int d1 = digit(timestamp[pos + 1]);
        if (d0 < 0 || d1 < 0) {
            throw std::runtime_error("Invalid ISO8601 timestamp");
        }
        return d0 * 10 + d1;
    };
    auto parse4 = [&](const size_t pos) -> int {
        return parse2(pos) * 100 + parse2(pos + 2);
    };

    if (timestamp[4] != '-' || timestamp[7] != '-' || (timestamp[10] != 'T' && timestamp[10] != ' ')
        || timestamp[13] != ':' || timestamp[16] != ':') {
        throw std::runtime_error("Invalid ISO8601 timestamp");
    }

    const int year = parse4(0);
    const int month = parse2(5);
    const int day = parse2(8);
    const int hour = parse2(11);
    const int minute = parse2(14);
    const int second = parse2(17);

    size_t pos = 19;
    int64_t nanos = 0;
    if (pos < timestamp.size() && timestamp[pos] == '.') {
        ++pos;
        int digits = 0;
        while (pos < timestamp.size()) {
            const int d = digit(timestamp[pos]);
            if (d < 0) {
                break;
            }
            if (digits < 9) {
                nanos = nanos * 10 + d;
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
