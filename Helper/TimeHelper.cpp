//
// Created by hugo on 15/03/25.
//

#include "TimeHelper.h"

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


