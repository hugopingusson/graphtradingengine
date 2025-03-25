//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_LOGGER_H
#define CLASS_LOGGER_H
#include <string>
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace std;
using namespace boost::filesystem;



class Logger {
    public:
    Logger();
    // Logger(const std::string& log_folder);


    void log_info(const string& msg);
    void log_error(const string& msg);



    protected:
    string log_root;
    string log_folder;
    std::shared_ptr<spdlog::logger> spdlogger;


};




#endif //CLASS_LOGGER_H
