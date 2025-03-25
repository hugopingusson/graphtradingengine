//
// Created by hugo on 15/03/25.
//


#include "DataReader.h"

DataReader::DataReader():database_root("/media/hugo/T7/market_data/databento/mbp10"){};
DataReader::DataReader(const string& database_root):database_root(database_root){};



std::shared_ptr<arrow::Table> DataReader::get_cme_market_data_table(const string& ticker,const string& date) {

    FutureHelper future_helper=FutureHelper(ticker);
    string liquid_contract = future_helper.get_liquid_contract(date);

    path base_dir(this->database_root);
    path contract_dir(liquid_contract);
    path file(fmt::format("databento_order_book_mbp10_cme_{}_{}.parquet",liquid_contract,date));
    path full_path = base_dir / contract_dir / file;

    std::shared_ptr<arrow::io::ReadableFile> infile;
    auto status = arrow::io::ReadableFile::Open(full_path.string(), arrow::default_memory_pool()).Value(&infile);
    std::unique_ptr<parquet::arrow::FileReader> reader;
    status = parquet::arrow::OpenFile(infile, arrow::default_memory_pool(), &reader);
    std::shared_ptr<arrow::Table> table;
    status = reader->ReadTable(&table);

    return table;



}
