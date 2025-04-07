//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_LOGGER_H
#define CLASS_LOGGER_H
#include <string>
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include "../Helper/TimeHelper.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace boost::filesystem;



class Logger {
    public:
    Logger();
    Logger(const string& logger_name,const string& log_location);
    Logger(const string& logger_name,const string& log_location,const string& log_folder);


    void log_info(const string& component,const string& msg);
    void log_error(const string& component,const string& msg);



    protected:
    string log_location;
    string log_folder;
    string logger_name;
    // std::shared_ptr<spdlog::logger> spdlogger;
    std::shared_ptr<spdlog::logger> spdlogger;


};




#endif //CLASS_LOGGER_H
