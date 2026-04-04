//
// Created by hugo on 03/04/26.
//

#include "CryptolakeParquetToBin.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <parquet/arrow/reader.h>

#include "../../Data/DataStructure/DataStructure.h"

using boost::filesystem::path;
using std::array;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::vector;

namespace {
void throw_if_status_not_ok(const arrow::Status& status, const string& context) {
    if (!status.ok()) {
        throw std::runtime_error(fmt::format("{}: {}", context, status.ToString()));
    }
}

template <typename ArrayT>
shared_ptr<ArrayT> get_array_by_name(const shared_ptr<arrow::Table>& table, const string& name) {
    const auto chunked = table->GetColumnByName(name);
    if (!chunked) {
        throw std::runtime_error(fmt::format("Column '{}' not found in parquet table", name));
    }
    if (chunked->num_chunks() == 0) {
        throw std::runtime_error(fmt::format("Column '{}' has no chunks", name));
    }
    if (chunked->num_chunks() != 1) {
        throw std::runtime_error(
            fmt::format("Column '{}' has {} chunks; expected 1 chunk per row-group read", name, chunked->num_chunks())
        );
    }

    auto typed = std::dynamic_pointer_cast<ArrayT>(chunked->chunk(0));
    if (!typed) {
        throw std::runtime_error(fmt::format("Column '{}' has unexpected Arrow type", name));
    }
    return typed;
}

bool is_digit_ascii(const char c) {
    return c >= '0' && c <= '9';
}

int parse_2digits(const string_view value, const size_t pos) {
    if (pos + 1 >= value.size() || !is_digit_ascii(value[pos]) || !is_digit_ascii(value[pos + 1])) {
        throw std::runtime_error(fmt::format("Invalid timestamp '{}'", string(value)));
    }
    return (value[pos] - '0') * 10 + (value[pos + 1] - '0');
}

int parse_4digits(const string_view value, const size_t pos) {
    if (pos + 3 >= value.size()
        || !is_digit_ascii(value[pos])
        || !is_digit_ascii(value[pos + 1])
        || !is_digit_ascii(value[pos + 2])
        || !is_digit_ascii(value[pos + 3])) {
        throw std::runtime_error(fmt::format("Invalid timestamp '{}'", string(value)));
    }
    return (value[pos] - '0') * 1000
        + (value[pos + 1] - '0') * 100
        + (value[pos + 2] - '0') * 10
        + (value[pos + 3] - '0');
}

// Howard Hinnant's civil date to days conversion (days since 1970-01-01).
int64_t days_from_civil(int year, const unsigned month, const unsigned day) {
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned m_adj = month > 2 ? month - 3U : month + 9U;
    const unsigned doy = (153U * m_adj + 2U) / 5U + day - 1U;
    const unsigned doe = yoe * 365U + yoe / 4U - yoe / 100U + doy;
    return static_cast<int64_t>(era) * 146097LL + static_cast<int64_t>(doe) - 719468LL;
}

int64_t parse_timestamp_to_ns(const string_view value) {
    if (value.size() < 19
        || value[4] != '-'
        || value[7] != '-'
        || value[10] != ' '
        || value[13] != ':'
        || value[16] != ':') {
        throw std::runtime_error(fmt::format("Invalid timestamp '{}'", string(value)));
    }

    const int year = parse_4digits(value, 0);
    const int month = parse_2digits(value, 5);
    const int day = parse_2digits(value, 8);
    const int hour = parse_2digits(value, 11);
    const int minute = parse_2digits(value, 14);
    const int second = parse_2digits(value, 17);

    int64_t nanoseconds = 0;
    if (value.size() > 19) {
        if (value[19] != '.') {
            throw std::runtime_error(fmt::format("Invalid timestamp '{}'", string(value)));
        }
        int digits = 0;
        for (size_t i = 20; i < value.size(); ++i) {
            if (!is_digit_ascii(value[i])) {
                throw std::runtime_error(fmt::format("Invalid timestamp '{}'", string(value)));
            }
            if (digits < 9) {
                nanoseconds = nanoseconds * 10 + static_cast<int64_t>(value[i] - '0');
                ++digits;
            }
        }
        while (digits < 9) {
            nanoseconds *= 10;
            ++digits;
        }
    }

    const int64_t days = days_from_civil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
    const int64_t day_seconds = static_cast<int64_t>(hour) * 3600LL
        + static_cast<int64_t>(minute) * 60LL
        + static_cast<int64_t>(second);
    const int64_t epoch_seconds = days * 86400LL + day_seconds;
    return epoch_seconds * 1000000000LL + nanoseconds;
}

vector<string> build_required_column_names() {
    vector<string> columns = {"exchange_time", "reception_time"};
    columns.reserve(2 + 4 * kBookLevels);
    for (size_t level = 0; level < kBookLevels; ++level) {
        columns.push_back(fmt::format("bid_{}_price", level));
        columns.push_back(fmt::format("bid_{}_size", level));
        columns.push_back(fmt::format("ask_{}_price", level));
        columns.push_back(fmt::format("ask_{}_size", level));
    }
    return columns;
}

bool is_date_yyyy_mm_dd(const string& value) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
        return false;
    }
    for (size_t i = 0; i < value.size(); ++i) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (!is_digit_ascii(value[i])) {
            return false;
        }
    }
    return true;
}

string extract_date_from_parquet_filename(const path& parquet_path) {
    const string stem = parquet_path.stem().string();
    if (is_date_yyyy_mm_dd(stem)) {
        return stem;
    }

    const size_t last_underscore = stem.rfind('_');
    if (last_underscore == string::npos || last_underscore + 1 >= stem.size()) {
        throw std::runtime_error(fmt::format(
            "Cannot extract date from parquet filename '{}'",
            parquet_path.filename().string()
        ));
    }

    const string date = stem.substr(last_underscore + 1);
    if (!is_date_yyyy_mm_dd(date)) {
        throw std::runtime_error(fmt::format(
            "Invalid date '{}' extracted from parquet filename '{}'",
            date,
            parquet_path.filename().string()
        ));
    }
    return date;
}

string extract_exchange_from_parquet_filename(const path& parquet_path) {
    const string stem = parquet_path.stem().string();
    const string prefix = "cryptolake_order_book_snapshot_";
    if (stem.rfind(prefix, 0) != 0) {
        throw std::runtime_error(fmt::format(
            "Unexpected Cryptolake parquet filename '{}'",
            parquet_path.filename().string()
        ));
    }

    const string remainder = stem.substr(prefix.size()); // exchange_symbol_date
    const size_t first_underscore = remainder.find('_');
    const size_t last_underscore = remainder.rfind('_');
    if (first_underscore == string::npos || last_underscore == string::npos || first_underscore == last_underscore) {
        throw std::runtime_error(fmt::format(
            "Cannot extract exchange from parquet filename '{}'",
            parquet_path.filename().string()
        ));
    }

    return remainder.substr(0, first_underscore);
}

string extract_instrument_from_parquet_path(const path& parquet_path) {
    const path instrument_dir = parquet_path.parent_path().filename();
    if (instrument_dir.empty()) {
        throw std::runtime_error(fmt::format(
            "Cannot extract instrument from parquet path '{}'",
            parquet_path.string()
        ));
    }
    return instrument_dir.string();
}
}

CryptolakeParquetToBin::CryptolakeParquetToBin()
    : parquet_root("/media/hugo/T7/market_data/cryptolake/order_book"),
      bin_root("/media/hugo/T7/market_data/cryptolake/order_book_bin") {}

CryptolakeParquetToBin::CryptolakeParquetToBin(string parquet_root, string bin_root)
    : parquet_root(std::move(parquet_root)), bin_root(std::move(bin_root)) {}

void CryptolakeParquetToBin::convert_all_to_bin() const {
    const path input_root(this->parquet_root);
    const path output_root(this->bin_root);

    if (!boost::filesystem::exists(input_root)) {
        throw std::runtime_error(fmt::format("Parquet root does not exist: {}", this->parquet_root));
    }

    boost::filesystem::recursive_directory_iterator end_it;
    for (boost::filesystem::recursive_directory_iterator it(input_root); it != end_it; ++it) {
        if (!boost::filesystem::is_regular_file(it->path())) {
            continue;
        }
        if (it->path().extension() != ".parquet") {
            continue;
        }

        const string date = extract_date_from_parquet_filename(it->path());
        const string exchange = extract_exchange_from_parquet_filename(it->path());
        const string instrument = extract_instrument_from_parquet_path(it->path());
        const path out_path = output_root / path(exchange) / path(instrument) / path(fmt::format("{}.bin", date));
        this->convert_file_to_bin(it->path().string(), out_path.string());
    }
}

void CryptolakeParquetToBin::convert_to_bin(const string& symbol, const string& exchange, const string& date) const {
    const path parquet_path = path(this->parquet_root)
        / path(symbol)
        / path(fmt::format("cryptolake_order_book_snapshot_{}_{}_{}.parquet", exchange, symbol, date));
    const path bin_path = path(this->bin_root)
        / path(exchange)
        / path(symbol)
        / path(fmt::format("{}.bin", date));

    this->convert_file_to_bin(parquet_path.string(), bin_path.string());
}

void CryptolakeParquetToBin::convert_file_to_bin(const string& parquet_file_path, const string& bin_file_path) const {
    const path parquet_path(parquet_file_path);
    const path requested_bin_path(bin_file_path);

    if (!boost::filesystem::exists(parquet_path)) {
        throw std::runtime_error(fmt::format("Parquet file does not exist: {}", parquet_file_path));
    }

    const string date = extract_date_from_parquet_filename(parquet_path);
    const path bin_path = requested_bin_path.parent_path() / path(fmt::format("{}.bin", date));
    std::cout << fmt::format("[CryptolakeParquetToBin] {} -> {}", parquet_path.string(), bin_path.string()) << std::endl;

    const path out_dir = bin_path.parent_path();
    if (!out_dir.empty() && !boost::filesystem::exists(out_dir)) {
        boost::filesystem::create_directories(out_dir);
    }

    std::ofstream output(bin_path.string(), std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        throw std::runtime_error(fmt::format("Cannot open output .bin file: {}", bin_path.string()));
    }

    auto readable_result = arrow::io::ReadableFile::Open(parquet_file_path, arrow::default_memory_pool());
    if (!readable_result.ok()) {
        throw std::runtime_error(fmt::format("Cannot open parquet file '{}': {}", parquet_file_path, readable_result.status().ToString()));
    }
    shared_ptr<arrow::io::ReadableFile> infile = *readable_result;

    auto reader_result = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
    if (!reader_result.ok()) {
        throw std::runtime_error(
            fmt::format("Cannot create parquet reader for '{}': {}", parquet_file_path, reader_result.status().ToString())
        );
    }
    std::unique_ptr<parquet::arrow::FileReader> reader = std::move(*reader_result);

    shared_ptr<arrow::Schema> schema;
    throw_if_status_not_ok(reader->GetSchema(&schema), "Cannot read parquet schema");

    const vector<string> required_columns = build_required_column_names();
    vector<int> column_indices;
    column_indices.reserve(required_columns.size());
    for (const auto& column_name : required_columns) {
        const int idx = schema->GetFieldIndex(column_name);
        if (idx < 0) {
            throw std::runtime_error(fmt::format("Missing required parquet column '{}'", column_name));
        }
        column_indices.push_back(idx);
    }

    array<string, kBookLevels> bid_price_cols{};
    array<string, kBookLevels> bid_size_cols{};
    array<string, kBookLevels> ask_price_cols{};
    array<string, kBookLevels> ask_size_cols{};
    for (size_t level = 0; level < kBookLevels; ++level) {
        bid_price_cols[level] = fmt::format("bid_{}_price", level);
        bid_size_cols[level] = fmt::format("bid_{}_size", level);
        ask_price_cols[level] = fmt::format("ask_{}_price", level);
        ask_size_cols[level] = fmt::format("ask_{}_size", level);
    }

    const int row_group_count = reader->num_row_groups();
    for (int rg = 0; rg < row_group_count; ++rg) {
        shared_ptr<arrow::Table> table;
        throw_if_status_not_ok(
            reader->ReadRowGroup(rg, column_indices, &table),
            fmt::format("Cannot read row group {} from '{}'", rg, parquet_file_path)
        );

        if (!table || table->num_rows() == 0) {
            continue;
        }

        auto exchange_time_arr = get_array_by_name<arrow::StringArray>(table, "exchange_time");
        auto reception_time_arr = get_array_by_name<arrow::StringArray>(table, "reception_time");

        array<shared_ptr<arrow::DoubleArray>, kBookLevels> bid_price_arr{};
        array<shared_ptr<arrow::DoubleArray>, kBookLevels> bid_size_arr{};
        array<shared_ptr<arrow::DoubleArray>, kBookLevels> ask_price_arr{};
        array<shared_ptr<arrow::DoubleArray>, kBookLevels> ask_size_arr{};
        for (size_t level = 0; level < kBookLevels; ++level) {
            bid_price_arr[level] = get_array_by_name<arrow::DoubleArray>(table, bid_price_cols[level]);
            bid_size_arr[level] = get_array_by_name<arrow::DoubleArray>(table, bid_size_cols[level]);
            ask_price_arr[level] = get_array_by_name<arrow::DoubleArray>(table, ask_price_cols[level]);
            ask_size_arr[level] = get_array_by_name<arrow::DoubleArray>(table, ask_size_cols[level]);
        }

        const int64_t row_count = table->num_rows();
        for (int64_t i = 0; i < row_count; ++i) {
            if (exchange_time_arr->IsNull(i) || reception_time_arr->IsNull(i)) {
                throw std::runtime_error(fmt::format("Null exchange_time/reception_time at row {} in '{}'", i, parquet_file_path));
            }

            WideMarketByPriceMessage row{};

            const auto exchange_time_view = exchange_time_arr->GetView(i);
            const auto reception_time_view = reception_time_arr->GetView(i);
            row.market_time_stamp.order_gateway_in_timestamp =
                parse_timestamp_to_ns(string_view(exchange_time_view.data(), exchange_time_view.size()));
            row.market_time_stamp.capture_server_in_timestamp =
                parse_timestamp_to_ns(string_view(reception_time_view.data(), reception_time_view.size()));
            row.market_time_stamp.data_gateway_out_timestamp = row.market_time_stamp.capture_server_in_timestamp;

            row.order.side = Side::NEUTRAL;
            row.order.action = Action::ADD;
            row.order.layer = 0;
            row.order.price = 0.0;
            row.order.size = 0.0;

            for (size_t level = 0; level < kBookLevels; ++level) {
                row.order_book_snapshot_data.bid_price[level] = bid_price_arr[level]->Value(i);
                row.order_book_snapshot_data.bid_size[level] = bid_size_arr[level]->Value(i);
                row.order_book_snapshot_data.ask_price[level] = ask_price_arr[level]->Value(i);
                row.order_book_snapshot_data.ask_size[level] = ask_size_arr[level]->Value(i);
            }

            output.write(reinterpret_cast<const char*>(&row), sizeof(row));
            if (!output.good()) {
                throw std::runtime_error(fmt::format("Failed to write output row {} in '{}'", i, bin_path.string()));
            }
        }
    }

    output.flush();
}
