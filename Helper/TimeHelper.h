//
// Created by hugo on 15/03/25.
//

#ifndef CLASS_DATEHELPER_H
#define CLASS_DATEHELPER_H

#include <ctime>
#include <cstdint>
#include <chrono>
#include <string>

using namespace std::chrono;

using namespace std;
enum class TimeResolution : std::int32_t { seconds=0, nanoseconds=1};

class Date {
    public:
    Date(const int& year, const int& month, const int& day);

    int64_t unixtime() const;
    string to_string() const;

    private:
    int year, month, day;

};

class Time {
    public:
    Time(const int& hour, const int& minute, const int& second);

    int64_t unixtime() const;
    string to_string() const;

    private:
    int hour, minute, second;


};


class Timestamp {
    public:
    Timestamp(const Date& date, const Time& time);
    Timestamp(const string& string_timestamp);
    Timestamp(const int64_t& unix_timestamp);

    Date get_date() const;
    Time get_time() const;

    int64_t unixtime() const;
    static int64_t now_unix(const TimeResolution& time_resolution);
    static int64_t unix_to_string(const int64_t& unix_timestamp);


    private:
    Date date;
    Time time;
};




class TimeHelper
{
    public:
    TimeHelper();

    int64_t now_nanoseconds_resolution_int64();
    int64_t now_second_resolution_int64();

    string convert_nanoseconds_to_string(int64_t nanoseconds);



};



#endif //CLASS_DATEHELPER_H
