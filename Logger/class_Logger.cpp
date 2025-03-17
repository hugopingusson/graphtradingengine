//
// Created by hugo on 16/03/25.
//

#include "class_Logger.h"

Logger::Logger():log_root("/home/hugo/gte_logs") {
    auto const now = std::chrono::system_clock::now();
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
    path base_dir(this->log_root);
    path log_folder_("logs_"+std::to_string(epoch_time));
    path file("logs.log");
    log_folder = (base_dir / log_folder_).string();

    if (!boost::filesystem::exists(log_folder)) {
        boost::filesystem::create_directory(log_folder);
    }

    spdlogger = spdlog::basic_logger_mt("MainLogger", (base_dir / log_folder_/file).string());





}

void Logger::log_info(const string& msg) {
    this->spdlogger->info(msg);
}

void Logger::log_error(const string& msg) {
    this->spdlogger->error(msg);
}