//
// Created by hugo on 16/03/25.
//

#include "Logger.h"


Logger::Logger(){}

Logger::~Logger() {
    if (this->spdlogger) {
        this->spdlogger->flush();
    }
}

Logger::Logger(const string& logger_name,const string& log_location,const string& log_folder) {
    if (!boost::filesystem::exists(log_location)) {
        throw std::runtime_error(fmt::format("log location {} does not exist on the machine", log_location));
    }
    else {
        this->log_location=log_location;
    }

    path log_location_(log_location);
    path log_folder_(log_folder);

    if (!boost::filesystem::exists(log_location_ / log_folder_)) {
        boost::filesystem::create_directories(log_location_ / log_folder_);
    }

    this->log_folder=log_folder;
    this->logger_name=logger_name;

    path log_file_(fmt::format("{}.log",logger_name));

    auto sink =std::make_shared<spdlog::sinks::basic_file_sink_mt>((log_location_ / log_folder_ /log_file_).string());
    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(1024, 1);
    }
    spdlogger = std::make_shared<spdlog::async_logger>(logger_name, sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    this->spdlogger->set_level(spdlog::level::info);
    this->spdlogger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(spdlogger);


}


Logger::Logger(const string& logger_name,const string& log_location) {
    if (!boost::filesystem::exists(log_location)) {
        throw std::runtime_error(fmt::format("log location {} does not exist on the machine", log_location));
    }
    else {
        this->log_location=log_location;
    }

    path log_location_(log_location);
    this->log_folder=fmt::format("logs_{}",std::to_string(Timestamp::now_unix(TimeResolution::seconds)));
    path log_folder_(this->log_folder);


    if (!boost::filesystem::exists(log_location_ / log_folder_)) {
        boost::filesystem::create_directories(log_location_ / log_folder_);
    }

    this->logger_name=logger_name;

    path log_file_(fmt::format("{}.log",logger_name));

    auto sink =std::make_shared<spdlog::sinks::basic_file_sink_mt>((log_location_ / log_folder_ /log_file_).string());
    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(1024, 1);
    }
    spdlogger = std::make_shared<spdlog::async_logger>(logger_name, sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    this->spdlogger->set_level(spdlog::level::info);
    this->spdlogger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(spdlogger);


}



void Logger::log_info(const string& component,const string& msg) {
    this->spdlogger->info(fmt::format("[{}] {}",component,msg));
}

void Logger::log_error(const string& component,const string& msg) {
    this->spdlogger->error(fmt::format("[{}] {}",component,msg));
}

void Logger::log_warn(const string& component,const string& msg) {
    this->spdlogger->warn(fmt::format("[{}] {}",component,msg));
}

void Logger::throw_error(const string& component,const string& msg) {
    this->spdlogger->error("[{}] {}",component,msg);
    throw std::runtime_error(fmt::format("[{}] {}",component,msg));
}

// Logger::Logger(const string& logger_name,const string& log_location){
//     this->log_location=log_location;
//     auto const now = std::chrono::system_clock::now();
//     std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
//     path base_dir(this->log_location);
//     path log_folder_("logs_"+std::to_string(epoch_time));
//     path file("logs.log");
//     log_folder = (base_dir / log_folder_).string();
//
//     if (!boost::filesystem::exists(log_folder)) {
//         boost::filesystem::create_directory(log_folder);
//     }
//     spdlogger = spdlog::basic_logger_mt(logger_name, (base_dir / log_folder_/file).string());
//
// }


// Logger::Logger(const string& logger_name,const string& log_location){
//     this->log_location=log_location;
//     auto const now = std::chrono::system_clock::now();
//     std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
//     path base_dir(this->log_location);
//     path log_folder_("logs_"+std::to_string(epoch_time));
//     path file("logs.log");
//     log_folder = (base_dir / log_folder_).string();
//
//     if (!boost::filesystem::exists(log_folder)) {
//         boost::filesystem::create_directory(log_folder);
//     }
//     spdlogger = spdlog::basic_logger_mt(logger_name, (base_dir / log_folder_/file).string());
//
// }
