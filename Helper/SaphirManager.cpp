//
// Created by hugo on 03/04/26.
//

#include "SaphirManager.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <unordered_set>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fmt/core.h>

using boost::property_tree::ptree;
using std::map;
using std::string;
using std::unordered_set;
namespace fs = std::filesystem;

namespace {
string expand_user_path(const string& raw_path) {
    if (raw_path.empty()) {
        throw std::runtime_error("Saphir root path cannot be empty");
    }
    if (raw_path[0] != '~') {
        return raw_path;
    }

    const char* home = std::getenv("HOME");
    if (!home || string(home).empty()) {
        throw std::runtime_error("HOME environment variable is not set");
    }

    if (raw_path == "~") {
        return string(home);
    }
    if (raw_path.size() >= 2 && raw_path[1] == '/') {
        return string(home) + raw_path.substr(1);
    }

    throw std::runtime_error(fmt::format("Unsupported user path format '{}'", raw_path));
}

unordered_set<string> read_string_array(const ptree& root, const string& key, const string& config_path) {
    const auto array_node_opt = root.get_child_optional(key);
    if (!array_node_opt) {
        throw std::runtime_error(fmt::format("Missing key '{}' in {}", key, config_path));
    }

    unordered_set<string> out;
    for (const auto& item : *array_node_opt) {
        const auto value_opt = item.second.get_value_optional<string>();
        if (!value_opt) {
            throw std::runtime_error(fmt::format("Invalid array entry in key '{}' in {}", key, config_path));
        }
        out.insert(*value_opt);
    }
    return out;
}

template <typename UIntType>
UIntType read_positive_unsigned(const ptree& root, const string& key, const string& config_path) {
    const auto value_opt = root.get_optional<long long>(key);
    if (!value_opt) {
        throw std::runtime_error(fmt::format("Missing key '{}' in {}", key, config_path));
    }
    const long long value = *value_opt;
    if (value <= 0) {
        throw std::runtime_error(fmt::format("Key '{}' in {} must be > 0", key, config_path));
    }
    if (static_cast<unsigned long long>(value) > static_cast<unsigned long long>(std::numeric_limits<UIntType>::max())) {
        throw std::runtime_error(fmt::format("Key '{}' in {} is too large", key, config_path));
    }
    return static_cast<UIntType>(value);
}
}

SaphirManager::SaphirManager()
    : saphir_root(expand_user_path("~/Saphir")) {}

SaphirManager::SaphirManager(string saphir_root)
    : saphir_root(expand_user_path(std::move(saphir_root))) {}

string SaphirManager::get_saphir_root() const {
    return this->saphir_root;
}

string SaphirManager::get_database_config_path() const {
    return (fs::path(this->saphir_root) / "DatabaseConfig.json").string();
}

string SaphirManager::get_instrument_config_path() const {
    return (fs::path(this->saphir_root) / "InstrumentConfig.json").string();
}

string SaphirManager::get_live_engine_config_path() const {
    return (fs::path(this->saphir_root) / "LiveEngineConfig.json").string();
}

string SaphirManager::get_tick_config_path() const {
    return (fs::path(this->saphir_root) / "TickConfig.json").string();
}

void SaphirManager::initialize() const {
    this->ensure_root_exists();
    this->ensure_database_config_exists();
    this->ensure_instrument_config_exists();
    this->ensure_live_engine_config_exists();
    this->ensure_tick_config_exists();
}

void SaphirManager::ensure_root_exists() const {
    const fs::path root(this->saphir_root);
    if (!fs::exists(root)) {
        fs::create_directories(root);
    }
    if (!fs::is_directory(root)) {
        throw std::runtime_error(fmt::format("Saphir root '{}' is not a directory", this->saphir_root));
    }
}

void SaphirManager::ensure_database_config_exists() const {
    const fs::path config_path(this->get_database_config_path());
    if (fs::exists(config_path)) {
        return;
    }

    ptree root;
    ptree databases;
    databases.put("cme", "/media/hugo/T7/market_data_bin/chi/databento");
    databases.put("binance", "/media/hugo/T7/market_data_bin/nyk/cryptolake");
    databases.put("okx", "/media/hugo/T7/market_data_bin/nyk/cryptolake");
    databases.put("bitmex", "/media/hugo/T7/market_data_bin/nyk/cryptolake");
    databases.put("deribit", "/media/hugo/T7/market_data_bin/nyk/cryptolake");
    root.add_child("databases", databases);

    boost::property_tree::write_json(config_path.string(), root);
}

void SaphirManager::ensure_instrument_config_exists() const {
    const fs::path config_path(this->get_instrument_config_path());
    if (fs::exists(config_path)) {
        return;
    }

    ptree root;
    ptree futures;
    for (const auto* value : {"6E", "6J", "6B", "6A", "6C", "6N", "6S", "NOK", "SEK"}) {
        ptree node;
        node.put("", value);
        futures.push_back(std::make_pair("", node));
    }

    ptree cryptocurrencies;
    for (const auto* value : {
            "BTCUSDT", "ETHUSDT", "SOLUSDT", "BNBUSDT", "XRPUSDT", "ADAUSDT",
            "DOGEUSDT", "LTCUSDT", "AVAXUSDT", "DOTUSDT", "LINKUSDT", "TRXUSDT"}) {
        ptree node;
        node.put("", value);
        cryptocurrencies.push_back(std::make_pair("", node));
    }

    root.add_child("futures", futures);
    root.add_child("cryptocurrencies", cryptocurrencies);
    boost::property_tree::write_json(config_path.string(), root);
}

void SaphirManager::ensure_live_engine_config_exists() const {
    const fs::path config_path(this->get_live_engine_config_path());
    if (fs::exists(config_path)) {
        return;
    }

    ptree root;
    const fs::path legacy_path(fs::path(this->saphir_root) / "EngineConfig.json");
    if (fs::exists(legacy_path)) {
        boost::property_tree::read_json(legacy_path.string(), root);
    }

    if (!root.get_optional<long long>("market_depth_by_exchange.cme")) {
        root.put("market_depth_by_exchange.cme", 25);
    }
    if (!root.get_optional<long long>("market_depth_by_exchange.binance")) {
        root.put("market_depth_by_exchange.binance", 25);
    }
    if (!root.get_optional<long long>("market_depth_by_exchange.okx")) {
        root.put("market_depth_by_exchange.okx", 25);
    }
    if (!root.get_optional<long long>("market_depth_by_exchange.bitmex")) {
        root.put("market_depth_by_exchange.bitmex", 25);
    }
    if (!root.get_optional<long long>("market_depth_by_exchange.deribit")) {
        root.put("market_depth_by_exchange.deribit", 25);
    }

    if (!root.get_optional<long long>("ring_capacity")) {
        root.put("ring_capacity", 16384);
    }
    if (!root.get_optional<long long>("max_update_batch_size")) {
        root.put("max_update_batch_size", 64);
    }
    if (!root.get_optional<string>("logger_mode")) {
        root.put("logger_mode", "live");
    }

    if (!root.get_child_optional("supported_live_exchange")) {
        ptree exchanges;
        for (const auto* exchange : {"binance", "okx", "bitmex", "deribit"}) {
            ptree node;
            node.put("", exchange);
            exchanges.push_back(std::make_pair("", node));
        }
        root.add_child("supported_live_exchange", exchanges);
    }

    boost::property_tree::write_json(config_path.string(), root);
}

void SaphirManager::ensure_tick_config_exists() const {
    const fs::path config_path(this->get_tick_config_path());
    if (fs::exists(config_path)) {
        return;
    }

    ptree root;
    ptree exchanges;
    exchanges.add_child("cme", ptree{});
    exchanges.add_child("binance", ptree{});
    exchanges.add_child("okx", ptree{});
    exchanges.add_child("bitmex", ptree{});
    exchanges.add_child("deribit", ptree{});
    root.add_child("tick_values", exchanges);
    boost::property_tree::write_json(config_path.string(), root);
}

string SaphirManager::normalize_exchange(const string& exchange) const {
    string normalized = exchange;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (normalized != "cme"
        && normalized != "binance"
        && normalized != "okx"
        && normalized != "bitmex"
        && normalized != "deribit") {
        throw std::runtime_error(fmt::format(
            "Unsupported exchange '{}'. Allowed: cme, binance, okx, bitmex, deribit",
            exchange
        ));
    }
    return normalized;
}

string SaphirManager::get_database_root(const string& exchange) const {
    this->initialize();
    const string normalized_exchange = normalize_exchange(exchange);

    ptree root;
    boost::property_tree::read_json(this->get_database_config_path(), root);

    const string key = fmt::format("databases.{}", normalized_exchange);
    const auto value = root.get_optional<string>(key);
    if (!value) {
        throw std::runtime_error(fmt::format("Missing key '{}' in {}", key, this->get_database_config_path()));
    }
    return *value;
}

void SaphirManager::set_database_root(const string& exchange, const string& root_path) const {
    this->initialize();
    const string normalized_exchange = normalize_exchange(exchange);

    ptree root;
    boost::property_tree::read_json(this->get_database_config_path(), root);
    root.put(fmt::format("databases.{}", normalized_exchange), root_path);
    boost::property_tree::write_json(this->get_database_config_path(), root);
}

map<string, string> SaphirManager::get_all_database_roots() const {
    this->initialize();
    ptree root;
    boost::property_tree::read_json(this->get_database_config_path(), root);

    map<string, string> out;
    out["cme"] = root.get<string>("databases.cme");
    out["binance"] = root.get<string>("databases.binance");
    out["okx"] = root.get<string>("databases.okx");
    out["bitmex"] = root.get<string>("databases.bitmex", "");
    out["deribit"] = root.get<string>("databases.deribit", "");
    return out;
}

unordered_set<string> SaphirManager::get_futures_instruments() const {
    this->initialize();
    const string config_path = this->get_instrument_config_path();

    ptree root;
    boost::property_tree::read_json(config_path, root);
    return read_string_array(root, "futures", config_path);
}

unordered_set<string> SaphirManager::get_cryptocurrencies_instruments() const {
    this->initialize();
    const string config_path = this->get_instrument_config_path();

    ptree root;
    boost::property_tree::read_json(config_path, root);
    return read_string_array(root, "cryptocurrencies", config_path);
}

unordered_set<string> SaphirManager::get_all_instruments() const {
    unordered_set<string> all = this->get_futures_instruments();
    const unordered_set<string> cryptos = this->get_cryptocurrencies_instruments();
    all.insert(cryptos.begin(), cryptos.end());
    return all;
}

unordered_set<string> SaphirManager::get_supported_live_exchanges() const {
    this->initialize();
    const string config_path = this->get_live_engine_config_path();
    ptree root;
    boost::property_tree::read_json(config_path, root);

    const unordered_set<string> configured = read_string_array(root, "supported_live_exchange", config_path);
    unordered_set<string> supported;
    for (const auto& exchange : configured) {
        const string normalized = this->normalize_exchange(exchange);
        if (normalized == "cme") {
            throw std::runtime_error(fmt::format(
                "Exchange '{}' cannot be listed in supported_live_exchange in {}",
                exchange,
                config_path
            ));
        }
        supported.insert(normalized);
    }
    return supported;
}

size_t SaphirManager::get_market_depth(const string& exchange) const {
    this->initialize();
    const string normalized_exchange = normalize_exchange(exchange);
    const string config_path = this->get_live_engine_config_path();
    ptree root;
    boost::property_tree::read_json(config_path, root);
    const string key = fmt::format("market_depth_by_exchange.{}", normalized_exchange);
    return read_positive_unsigned<size_t>(root, key, config_path);
}

size_t SaphirManager::get_ring_capacity() const {
    this->initialize();
    const string config_path = this->get_live_engine_config_path();
    ptree root;
    boost::property_tree::read_json(config_path, root);
    return read_positive_unsigned<size_t>(root, "ring_capacity", config_path);
}

size_t SaphirManager::get_max_update_batch_size() const {
    this->initialize();
    const string config_path = this->get_live_engine_config_path();
    ptree root;
    boost::property_tree::read_json(config_path, root);
    return read_positive_unsigned<size_t>(root, "max_update_batch_size", config_path);
}

double SaphirManager::get_market_tick_value(const string& exchange, const string& instrument) const {
    this->initialize();
    const string config_path = this->get_tick_config_path();
    const string normalized_exchange = normalize_exchange(exchange);
    if (instrument.empty()) {
        throw std::runtime_error(fmt::format("Instrument cannot be empty for tick lookup in {}", config_path));
    }

    ptree root;
    boost::property_tree::read_json(config_path, root);

    const string key = fmt::format("tick_values.{}.{}", normalized_exchange, instrument);
    const auto tick_value_opt = root.get_optional<double>(key);
    if (!tick_value_opt) {
        throw std::runtime_error(fmt::format(
            "Missing tick value key '{}' in {}. Configure tick_values by exchange/instrument.",
            key,
            config_path
        ));
    }
    if (!std::isfinite(*tick_value_opt) || *tick_value_opt <= 0.0) {
        throw std::runtime_error(fmt::format(
            "Key '{}' in {} must be a finite positive number",
            key,
            config_path
        ));
    }
    return *tick_value_opt;
}

string SaphirManager::get_logger_mode() const {
    this->initialize();
    const string config_path = this->get_live_engine_config_path();
    ptree root;
    boost::property_tree::read_json(config_path, root);
    const auto mode_opt = root.get_optional<string>("logger_mode");
    if (!mode_opt) {
        throw std::runtime_error(fmt::format("Missing key '{}' in {}", "logger_mode", config_path));
    }

    string mode = *mode_opt;
    std::transform(mode.begin(), mode.end(), mode.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (mode != "live" && mode != "debug") {
        throw std::runtime_error(fmt::format("Unsupported logger_mode '{}' in {}. Allowed: live, debug", *mode_opt, config_path));
    }
    return mode;
}
