//
// Created by hugo on 16/03/25.
//

#include "Logger.h"

#include "../Helper/SaphirManager.h"

namespace {

LoggerMode resolve_mode_from_saphir() {
    try {
        const SaphirManager saphir;
        const std::string mode = saphir.get_logger_mode();
        if (mode == "live") {
            return LoggerMode::LIVE;
        }
        return LoggerMode::DEBUG;
    } catch (...) {
        return LoggerMode::DEBUG;
    }
}
}

Logger::Logger()
    : log_location(),
      log_folder(),
      logger_name(),
      mode(LoggerMode::DEBUG),
      spdlogger(nullptr) {}

Logger::~Logger() {
    if (this->spdlogger) {
        this->spdlogger->flush();
    }
}

Logger::Logger(const string& logger_name,const string& log_location,const string& log_folder)
    : Logger(logger_name, log_location, log_folder, resolve_mode_from_saphir()) {}

Logger::Logger(const string& logger_name,const string& log_location,const string& log_folder,LoggerMode mode)
    : Logger() {
    if (!boost::filesystem::exists(log_location)) {
        throw std::runtime_error(fmt::format("log location {} does not exist on the machine", log_location));
    }

    this->log_location = log_location;
    this->log_folder = log_folder;
    this->logger_name = logger_name;
    this->mode = mode;

    path log_location_path(log_location);
    path log_folder_path(log_folder);
    if (!boost::filesystem::exists(log_location_path / log_folder_path)) {
        boost::filesystem::create_directories(log_location_path / log_folder_path);
    }

    path log_file(fmt::format("{}.log", logger_name));
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>((log_location_path / log_folder_path / log_file).string());
    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(1024, 1);
    }
    this->spdlogger = std::make_shared<spdlog::async_logger>(
        logger_name,
        sink,
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );
    this->spdlogger->set_level(spdlog::level::info);
    if (this->mode == LoggerMode::LIVE) {
        this->spdlogger->flush_on(spdlog::level::err);
    } else {
        this->spdlogger->flush_on(spdlog::level::info);
    }
    spdlog::set_default_logger(this->spdlogger);
}


Logger::Logger(const string& logger_name,const string& log_location)
    : Logger(logger_name, log_location, resolve_mode_from_saphir()) {}

Logger::Logger(const string& logger_name,const string& log_location,LoggerMode mode)
    : Logger(logger_name,
             log_location,
             fmt::format("logs_{}", std::to_string(Timestamp::now_unix(TimeResolution::seconds))),
             mode) {
}



void Logger::log_info(const string& component,const string& msg) {
    this->spdlogger->info("[{}] {}", component, msg);
}

void Logger::log_error(const string& component,const string& msg) {
    this->spdlogger->error("[{}] {}", component, msg);
}

void Logger::log_warn(const string& component,const string& msg) {
    this->spdlogger->warn("[{}] {}", component, msg);
}

void Logger::flush() {
    if (this->spdlogger) {
        this->spdlogger->flush();
    }
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
