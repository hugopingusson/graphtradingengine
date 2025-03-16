//
// Created by hugo on 16/03/25.
//

#include "class_Logger.h"

Logger::Logger():log_root("~/gte_logs") {
    boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
    std::time_t epoch_time = boost::posix_time::to_time_t(current_time);
    path base_dir(this->log_root);
    path log_folder_("logs_"+std::to_string(epoch_time));
    log_folder = (base_dir / log_folder_).string();

    if (!boost::filesystem::exists(log_folder)) {
        boost::filesystem::create_directory(log_folder);
    }

    boost::log::add_file_log(
    boost::log::keywords::file_name = log_folder + "/logfile_%Y-%m-%d_%H-%M-%S.log", // File path with folder
    boost::log::keywords::rotation_size = 10 * 1024 * 1024,  // Rotate log file after 10 MB
    boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"  // Log format
    );
}

void Logger::log_info(const string& msg) {
    BOOST_LOG_TRIVIAL(info) << msg;
}

void Logger::log_error(const string& msg) {
    BOOST_LOG_TRIVIAL(error) << msg;
}