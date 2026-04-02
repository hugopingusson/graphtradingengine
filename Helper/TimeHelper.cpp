//
// Created by hugo on 15/03/25.
//

#include "TimeHelper.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std::chrono;

namespace {
bool is_leap_year(const int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(const int year, const int month) {
    static const int kDaysByMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    return kDaysByMonth[month - 1];
}

bool is_ascii_digit(const char c) {
    return c >= '0' && c <= '9';
}

int parse_2digits(const string& value, const size_t start) {
    if (start + 1 >= value.size() || !is_ascii_digit(value[start]) || !is_ascii_digit(value[start + 1])) {
        throw std::invalid_argument("Invalid timestamp format");
    }
    return (value[start] - '0') * 10 + (value[start + 1] - '0');
}

int parse_4digits(const string& value, const size_t start) {
    if (start + 3 >= value.size()
        || !is_ascii_digit(value[start])
        || !is_ascii_digit(value[start + 1])
        || !is_ascii_digit(value[start + 2])
        || !is_ascii_digit(value[start + 3])) {
        throw std::invalid_argument("Invalid timestamp format");
    }
    return (value[start] - '0') * 1000
        + (value[start + 1] - '0') * 100
        + (value[start + 2] - '0') * 10
        + (value[start + 3] - '0');
}

uint64_t magnitude_u64(const int64_t value) {
    if (value >= 0) {
        return static_cast<uint64_t>(value);
    }
    return static_cast<uint64_t>(-(value + 1)) + 1ULL;
}

int64_t unix_to_epoch_seconds(const int64_t unix_timestamp) {
    const uint64_t abs_ts = magnitude_u64(unix_timestamp);

    // Heuristic by order of magnitude:
    // s: 1e9, ms: 1e12, us: 1e15, ns: 1e18
    if (abs_ts < 100000000000ULL) { // < 1e11
        return unix_timestamp;
    }
    if (abs_ts < 100000000000000ULL) { // < 1e14
        return unix_timestamp / 1000LL;
    }
    if (abs_ts < 100000000000000000ULL) { // < 1e17
        return unix_timestamp / 1000000LL;
    }
    return unix_timestamp / 1000000000LL;
}
}

Date::Date(const int& year, const int& month, const int& day) : year(year), month(month), day(day) {
    if (month < 1 || month > 12) {
        throw std::invalid_argument("Invalid Date");
    }
    if (day < 1 || day > days_in_month(year, month)) {
        throw std::invalid_argument("Invalid Date");
    }
}

int64_t Date::unixtime() const {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;

    std::time_t epoch_seconds = std::mktime(&tm);
    if (epoch_seconds == static_cast<std::time_t>(-1)) {
        throw std::runtime_error("Date::unixtime failed");
    }
    return static_cast<int64_t>(epoch_seconds);
}

string Date::to_string() const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day;
    return oss.str();
}

Time::Time(const int& hour, const int& minute, const int& second) : hour(hour), minute(minute), second(second) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        throw std::invalid_argument("Invalid Time");
    }
}

int64_t Time::unixtime() const {
    return static_cast<int64_t>(hour) * 3600 + static_cast<int64_t>(minute) * 60 + static_cast<int64_t>(second);
}

string Time::to_string() const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hour << ":"
        << std::setw(2) << minute << ":"
        << std::setw(2) << second;
    return oss.str();
}

Timestamp::Timestamp(const Date& date, const Time& time) : date(date), time(time) {}

Timestamp::Timestamp(const string& string_timestamp) : date(1970, 1, 1), time(0, 0, 0) {
    if (string_timestamp.size() != 19
        || string_timestamp[4] != '-'
        || string_timestamp[7] != '-'
        || string_timestamp[10] != ' '
        || string_timestamp[13] != ':'
        || string_timestamp[16] != ':') {
        throw std::invalid_argument("Invalid timestamp format, expected yyyy-mm-dd HH:MM:SS");
    }

    const int year = parse_4digits(string_timestamp, 0);
    const int month = parse_2digits(string_timestamp, 5);
    const int day = parse_2digits(string_timestamp, 8);
    const int hour = parse_2digits(string_timestamp, 11);
    const int minute = parse_2digits(string_timestamp, 14);
    const int second = parse_2digits(string_timestamp, 17);

    this->date = Date(year, month, day);
    this->time = Time(hour, minute, second);
}

Timestamp::Timestamp(const int64_t& unix_timestamp) : date(1970, 1, 1), time(0, 0, 0) {
    const int64_t epoch_seconds_i64 = unix_to_epoch_seconds(unix_timestamp);
    const std::time_t epoch_seconds = static_cast<std::time_t>(epoch_seconds_i64);
    std::tm* tm_ptr = std::localtime(&epoch_seconds);
    if (tm_ptr == nullptr) {
        throw std::runtime_error("Timestamp::Timestamp(unix) failed");
    }

    this->date = Date(tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday);
    this->time = Time(tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
}

int64_t Timestamp::unixtime() const {
    return this->date.unixtime() + this->time.unixtime();
}

int64_t Timestamp::now_unix(const TimeResolution& time_resolution) {
    const auto now = std::chrono::system_clock::now();
    if (time_resolution == TimeResolution::seconds) {
        return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
    }
    return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
}

int64_t Timestamp::unix_to_string(const int64_t& unix_timestamp) {
    const int64_t epoch_seconds_i64 = unix_to_epoch_seconds(unix_timestamp);
    const std::time_t epoch_seconds = static_cast<std::time_t>(epoch_seconds_i64);
    std::tm* tm_ptr = std::localtime(&epoch_seconds);
    if (tm_ptr == nullptr) {
        throw std::runtime_error("Timestamp::unix_to_string failed");
    }

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm_ptr->tm_year + 1900)
        << std::setw(2) << (tm_ptr->tm_mon + 1)
        << std::setw(2) << tm_ptr->tm_mday
        << std::setw(2) << tm_ptr->tm_hour
        << std::setw(2) << tm_ptr->tm_min
        << std::setw(2) << tm_ptr->tm_sec;
    return std::stoll(oss.str());
}

Date Timestamp::get_date() const {return this->date;}
Time Timestamp::get_time() const {return this->time;}


//
//
// TimeHelper::TimeHelper() {}
//
//
//
// int64_t TimeHelper::now_nanoseconds_resolution_int64() {
//     auto now = std::chrono::system_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
//     return (int64_t)duration.count();
// }
//
//
//
// int64_t TimeHelper::now_second_resolution_int64() {
//     auto now = std::chrono::system_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
//     return (int64_t)duration.count();
// }
//
//
// string TimeHelper::convert_nanoseconds_to_string(int64_t epoch) {
//     system_clock::time_point tp = time_point<system_clock, nanoseconds>(nanoseconds(epoch));
//     std::time_t time_t_epoch = system_clock::to_time_t(tp);
//     std::tm time_tm = *std::localtime(&time_t_epoch);
//     auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
//     // Step 4: Construct the final string (format: "YYYY-MM-DD HH:MM:SS.mmm")
//     char buffer[100];  // Ensure this buffer is big enough to hold the formatted string
//     std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_tm);
//
//     // Return the final string, append milliseconds
//     return std::string(buffer) + "." + std::to_string(ms.count());
// }
