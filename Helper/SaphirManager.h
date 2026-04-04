//
// Created by hugo on 03/04/26.
//

#ifndef SAPHIRMANAGER_H
#define SAPHIRMANAGER_H

#include <map>
#include <string>
#include <unordered_set>

class SaphirManager {
public:
    SaphirManager();
    explicit SaphirManager(std::string saphir_root);

    std::string get_saphir_root() const;
    std::string get_database_config_path() const;
    std::string get_instrument_config_path() const;

    // Ensure root folder and default JSON configs exist.
    void initialize() const;

    // Read configured root path for one exchange (cme, binance, okx).
    std::string get_database_root(const std::string& exchange) const;

    // Update configured root path for one exchange.
    void set_database_root(const std::string& exchange, const std::string& root_path) const;

    // Return all configured database roots.
    std::map<std::string, std::string> get_all_database_roots() const;

    // Accessors for InstrumentConfig.json
    std::unordered_set<std::string> get_futures_instruments() const;
    std::unordered_set<std::string> get_cryptocurrencies_instruments() const;
    std::unordered_set<std::string> get_all_instruments() const;

private:
    std::string saphir_root;

    void ensure_root_exists() const;
    void ensure_database_config_exists() const;
    void ensure_instrument_config_exists() const;
    static std::string normalize_exchange(const std::string& exchange);
};

#endif //SAPHIRMANAGER_H
