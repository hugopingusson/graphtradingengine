//
// Created by hugo on 15/03/25.
//

#ifndef CLASS_PARQUETDATAREADER_HEADER_H
#define CLASS_PARQUETDATAREADER_HEADER_H


#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <iostream>
#include <boost/date_time/gregorian/greg_date.hpp>
#include "../../Helper/FutureHelper.h"
#include <boost/filesystem.hpp>
#include <fmt/core.h>

using namespace std;
using namespace boost::filesystem;

class ParquetDataReader {

public:
    ParquetDataReader();
    ParquetDataReader(const string& database_root);

    std::shared_ptr<arrow::Table> get_cme_market_data_table(const string& ticker,const string& date);

    protected:
    const string database_root;

};
#endif //CLASS_PARQUETDATAREADER_HEADER_H

