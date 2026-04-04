//
// Created by hugo on 03/04/26.
//

#include "SaphirManager.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
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

void SaphirManager::initialize() const {
    this->ensure_root_exists();
    this->ensure_database_config_exists();
    this->ensure_instrument_config_exists();
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
    databases.put("cme", "/media/hugo/T7/market_data/databento/mbp10_bin");
    databases.put("binance", "/media/hugo/T7/market_data/cryptolake/order_book_bin");
    databases.put("okx", "/media/hugo/T7/market_data/cryptolake/order_book_bin");
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

string SaphirManager::normalize_exchange(const string& exchange) {
    string normalized = exchange;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (normalized != "cme" && normalized != "binance" && normalized != "okx") {
        throw std::runtime_error(fmt::format("Unsupported exchange '{}'. Allowed: cme, binance, okx", exchange));
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
