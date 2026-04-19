#include <exception>
#include <iostream>
#include <string>

#include "../Data/DataReader/CMEParquetToBin.h"
#include "../Data/DataReader/CryptolakeParquetToBin.h"

int main(int argc, char** argv) {
    std::string databento_parquet_root = "/media/hugo/T7/market_data/databento/mbp10";
    std::string databento_bin_root = "/media/hugo/T7/market_data_bin";
    std::string cryptolake_parquet_root = "/media/hugo/T7/market_data/cryptolake/order_book";
    std::string cryptolake_bin_root = "/media/hugo/T7/market_data_bin";

    if (argc == 5) {
        databento_parquet_root = argv[1];
        databento_bin_root = argv[2];
        cryptolake_parquet_root = argv[3];
        cryptolake_bin_root = argv[4];
    } else if (argc != 1) {
        std::cerr
            << "Usage:\n"
            << "  " << argv[0] << "\n"
            << "  " << argv[0] << " <databento_parquet_root> <databento_bin_root> <cryptolake_parquet_root> <cryptolake_bin_root>\n";
        return 2;
    }

    std::cout << "[MarketDataConverter] Databento input : " << databento_parquet_root << '\n';
    std::cout << "[MarketDataConverter] Databento output: " << databento_bin_root << '\n';
    std::cout << "[MarketDataConverter] Cryptolake input : " << cryptolake_parquet_root << '\n';
    std::cout << "[MarketDataConverter] Cryptolake output: " << cryptolake_bin_root << '\n';

    try {
        CMEParquetToBin cme_converter(databento_parquet_root, databento_bin_root);
        cme_converter.convert_all_to_bin();
        std::cout << "[MarketDataConverter] Databento conversion completed.\n";

        CryptolakeParquetToBin cryptolake_converter(cryptolake_parquet_root, cryptolake_bin_root);
        cryptolake_converter.convert_all_to_bin();
        std::cout << "[MarketDataConverter] Cryptolake conversion completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "[MarketDataConverter] Conversion failed: " << e.what() << '\n';
        return 1;
    }

    std::cout << "[MarketDataConverter] All conversions completed successfully.\n";
    return 0;
}
