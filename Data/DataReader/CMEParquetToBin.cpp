//
// Created by hugo on 03/04/26.
//

#include "CMEParquetToBin.h"

#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <vector>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <fmt/core.h>
#include <parquet/arrow/reader.h>
#include <boost/filesystem.hpp>

#include "../../Data/DataStructure/DataStructure.h"
#include "../../Helper/FutureHelper.h"

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

    if (chunked->num_chunks() == 1) {
        auto typed = std::dynamic_pointer_cast<ArrayT>(chunked->chunk(0));
        if (!typed) {
            throw std::runtime_error(fmt::format("Column '{}' has unexpected Arrow type", name));
        }
        return typed;
    }

    throw std::runtime_error(
        fmt::format("Column '{}' has {} chunks; expected 1 chunk per row-group read", name, chunked->num_chunks())
    );
}

char to_upper_ascii(const string_view value) {
    if (value.empty()) {
        return '\0';
    }
    return static_cast<char>(std::toupper(static_cast<unsigned char>(value[0])));
}

Action parse_action(const string_view action_value) {
    switch (to_upper_ascii(action_value)) {
        case 'A': return Action::ADD;
        case 'C': return Action::CANCEL;
        case 'M': return Action::MODIFY;
        case 'T': return Action::TRADE;
        default:
            throw std::runtime_error(fmt::format("Unsupported action value '{}'", string(action_value)));
    }
}

Side parse_side(const string_view side_value) {
    switch (to_upper_ascii(side_value)) {
        case 'B': return Side::BID;
        case 'A': return Side::ASK;
        case 'N': return Side::NEUTRAL;
        default:
            throw std::runtime_error(fmt::format("Unsupported side value '{}'", string(side_value)));
    }
}

vector<string> build_required_column_names() {
    vector<string> columns = {
        "ts_event",
        "ts_recv",
        "ts_in_delta",
        "action",
        "side",
        "depth",
        "price",
        "size"
    };

    columns.reserve(8 + 4 * kBookLevels);
    for (size_t level = 0; level < kBookLevels; ++level) {
        columns.push_back(fmt::format("bid_px_{:02d}", static_cast<int>(level)));
        columns.push_back(fmt::format("ask_px_{:02d}", static_cast<int>(level)));
        columns.push_back(fmt::format("bid_sz_{:02d}", static_cast<int>(level)));
        columns.push_back(fmt::format("ask_sz_{:02d}", static_cast<int>(level)));
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
        if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
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

CMEParquetToBin::CMEParquetToBin()
    : parquet_root("/media/hugo/T7/market_data/databento/mbp10"),
      bin_root("/media/hugo/T7/market_data/databento/mbp10_bin") {}

CMEParquetToBin::CMEParquetToBin(string parquet_root, string bin_root)
    : parquet_root(std::move(parquet_root)), bin_root(std::move(bin_root)) {}

void CMEParquetToBin::convert_all_to_bin() const {
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
        const string instrument = extract_instrument_from_parquet_path(it->path());
        const path out_path = output_root / path("cme") / path(instrument) / path(fmt::format("{}.bin", date));

        this->convert_file_to_bin(it->path().string(), out_path.string());
    }
}

void CMEParquetToBin::convert_to_bin(const string& instrument, const string& date) const {
    FutureHelper future_helper(instrument);
    const string contract = future_helper.get_liquid_contract(date);

    const path parquet_path = path(this->parquet_root)
        / path(contract)
        / path(fmt::format("databento_order_book_mbp10_cme_{}_{}.parquet", contract, date));
    const path bin_path = path(this->bin_root)
        / path("cme")
        / path(contract)
        / path(fmt::format("{}.bin", date));

    this->convert_file_to_bin(parquet_path.string(), bin_path.string());
}

void CMEParquetToBin::convert_file_to_bin(const string& parquet_file_path, const string& bin_file_path) const {
    const path parquet_path(parquet_file_path);
    const path requested_bin_path(bin_file_path);

    if (!boost::filesystem::exists(parquet_path)) {
        throw std::runtime_error(fmt::format("Parquet file does not exist: {}", parquet_file_path));
    }

    const string date = extract_date_from_parquet_filename(parquet_path);
    const path bin_path = requested_bin_path.parent_path() / path(fmt::format("{}.bin", date));
    std::cout << fmt::format("[CMEParquetToBin] {} -> {}", parquet_path.string(), bin_path.string()) << std::endl;

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

    const int row_group_count = reader->num_row_groups();
    array<string, kBookLevels> bid_px_columns{};
    array<string, kBookLevels> ask_px_columns{};
    array<string, kBookLevels> bid_sz_columns{};
    array<string, kBookLevels> ask_sz_columns{};
    for (size_t level = 0; level < kBookLevels; ++level) {
        bid_px_columns[level] = fmt::format("bid_px_{:02d}", static_cast<int>(level));
        ask_px_columns[level] = fmt::format("ask_px_{:02d}", static_cast<int>(level));
        bid_sz_columns[level] = fmt::format("bid_sz_{:02d}", static_cast<int>(level));
        ask_sz_columns[level] = fmt::format("ask_sz_{:02d}", static_cast<int>(level));
    }

    for (int rg = 0; rg < row_group_count; ++rg) {
        shared_ptr<arrow::Table> table;
        throw_if_status_not_ok(
            reader->ReadRowGroup(rg, column_indices, &table),
            fmt::format("Cannot read row group {} from '{}'", rg, parquet_file_path)
        );

        if (!table || table->num_rows() == 0) {
            continue;
        }

        auto ts_event_arr = get_array_by_name<arrow::TimestampArray>(table, "ts_event");
        auto ts_recv_arr = get_array_by_name<arrow::TimestampArray>(table, "ts_recv");
        auto ts_in_delta_arr = get_array_by_name<arrow::Int32Array>(table, "ts_in_delta");
        auto action_arr = get_array_by_name<arrow::StringArray>(table, "action");
        auto side_arr = get_array_by_name<arrow::StringArray>(table, "side");
        auto depth_arr = get_array_by_name<arrow::UInt8Array>(table, "depth");
        auto price_arr = get_array_by_name<arrow::DoubleArray>(table, "price");
        auto size_arr = get_array_by_name<arrow::UInt32Array>(table, "size");

        array<shared_ptr<arrow::DoubleArray>, kBookLevels> bid_px_arr{};
        array<shared_ptr<arrow::DoubleArray>, kBookLevels> ask_px_arr{};
        array<shared_ptr<arrow::UInt32Array>, kBookLevels> bid_sz_arr{};
        array<shared_ptr<arrow::UInt32Array>, kBookLevels> ask_sz_arr{};

        for (size_t level = 0; level < kBookLevels; ++level) {
            bid_px_arr[level] = get_array_by_name<arrow::DoubleArray>(table, bid_px_columns[level]);
            ask_px_arr[level] = get_array_by_name<arrow::DoubleArray>(table, ask_px_columns[level]);
            bid_sz_arr[level] = get_array_by_name<arrow::UInt32Array>(table, bid_sz_columns[level]);
            ask_sz_arr[level] = get_array_by_name<arrow::UInt32Array>(table, ask_sz_columns[level]);
        }

        const int64_t row_count = table->num_rows();
        for (int64_t i = 0; i < row_count; ++i) {
            if (action_arr->IsNull(i) || side_arr->IsNull(i)) {
                throw std::runtime_error(fmt::format("Null action/side encountered at row {} in '{}'", i, parquet_file_path));
            }

            BacktestWideMarketByPriceRow row{};

            row.market_time_stamp.order_gateway_in_timestamp = ts_event_arr->Value(i);
            row.capture_server_in_timestamp = ts_recv_arr->Value(i);
            row.market_time_stamp.data_gateway_out_timestamp =
                row.capture_server_in_timestamp - static_cast<int64_t>(ts_in_delta_arr->Value(i));

            const auto action_view = action_arr->GetView(i);
            const auto side_view = side_arr->GetView(i);
            row.order.action = parse_action(string_view(action_view.data(), action_view.size()));
            row.order.side = parse_side(string_view(side_view.data(), side_view.size()));
            row.order.layer = static_cast<int32_t>(depth_arr->Value(i));
            row.order.price = price_arr->Value(i);
            row.order.size = static_cast<double>(size_arr->Value(i));

            for (size_t level = 0; level < kBookLevels; ++level) {
                row.order_book_snapshot_data.bid_price[level] = bid_px_arr[level]->Value(i);
                row.order_book_snapshot_data.ask_price[level] = ask_px_arr[level]->Value(i);
                row.order_book_snapshot_data.bid_size[level] = static_cast<double>(bid_sz_arr[level]->Value(i));
                row.order_book_snapshot_data.ask_size[level] = static_cast<double>(ask_sz_arr[level]->Value(i));
            }

            output.write(reinterpret_cast<const char*>(&row), sizeof(row));
            if (!output.good()) {
                throw std::runtime_error(fmt::format("Failed to write output row {} in '{}'", i, bin_path.string()));
            }
        }
    }

    output.flush();
}
