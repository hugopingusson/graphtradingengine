//
// Created by hugo on 16/03/25.
//

#ifndef CLASS_LOGGER_H
#define CLASS_LOGGER_H
#include <string>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem.hpp>
#include <fmt/core.h>

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


};




#endif //CLASS_LOGGER_H
