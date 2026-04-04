//
// Created by hugo on 03/04/26.
//

#ifndef CMEPARQUETTOBIN_H
#define CMEPARQUETTOBIN_H

#include <string>

class CMEParquetToBin {
public:
    CMEParquetToBin();
    CMEParquetToBin(std::string parquet_root, std::string bin_root);

    // Convert every parquet file under parquet_root into a mirrored .bin tree under bin_root.
    void convert_all_to_bin() const;

    // Convert one (instrument, date) parquet into mirrored .bin output.
    void convert_to_bin(const std::string& instrument, const std::string& date) const;

    // Convert one explicit parquet file path into one explicit .bin file path.
    void convert_file_to_bin(const std::string& parquet_file_path, const std::string& bin_file_path) const;

private:
    std::string parquet_root;
    std::string bin_root;
};

#endif //CMEPARQUETTOBIN_H
