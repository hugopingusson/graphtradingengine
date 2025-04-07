//
// Created by hugo on 15/03/25.
//

#ifndef CLASS_DATEHELPER_H
#define CLASS_DATEHELPER_H

#include <ctime>
#include <cstdint>
#include <chrono>

using namespace std;

class TimeHelper
{
    public:
    TimeHelper();

    int64_t now_nanoseconds_resolution_int64();
    int64_t now_second_resolution_int64();




};



#endif //CLASS_DATEHELPER_H
