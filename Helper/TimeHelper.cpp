//
// Created by hugo on 15/03/25.
//

#include "TimeHelper.h"

#include <iomanip>
TimeHelper::TimeHelper() {}


int64_t TimeHelper::now_nanoseconds_resolution_int64() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    return (int64_t)duration.count();
}

int64_t TimeHelper::now_second_resolution_int64() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    return (int64_t)duration.count();
}


string TimeHelper::convert_nanoseconds_to_string(int64_t epoch) {
    system_clock::time_point tp = time_point<system_clock, nanoseconds>(nanoseconds(epoch));
    std::time_t time_t_epoch = system_clock::to_time_t(tp);
    std::tm time_tm = *std::localtime(&time_t_epoch);
    auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
    // Step 4: Construct the final string (format: "YYYY-MM-DD HH:MM:SS.mmm")
    char buffer[100];  // Ensure this buffer is big enough to hold the formatted string
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_tm);

    // Return the final string, append milliseconds
    return std::string(buffer) + "." + std::to_string(ms.count());
}

