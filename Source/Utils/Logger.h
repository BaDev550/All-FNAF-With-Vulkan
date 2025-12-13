#pragma once
#include <iostream>
#include <cassert>
#include <format>

#define ASSERT(x, ...) if(!(x)) LOG_CRITICAL("" ##__VA_ARGS__);
#define LOG_INFO(...)     logger::log_info(__VA_ARGS__);
#define LOG_WARN(...)     logger::log_warn(__VA_ARGS__);
#define LOG_ERROR(...)    logger::log_error(__VA_ARGS__);
#define LOG_CRITICAL(...) logger::log_critical(__VA_ARGS__);

namespace logger {
    template <typename... Args>
    static inline void log_info(std::string_view fmt, Args&&... args) { std::cout << "[INFO] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl; }

    template <typename... Args>
    static inline void log_warn(std::string_view fmt, Args&&... args) { std::cout << "[WARN] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl; }

    template <typename... Args>
    static inline void log_error(std::string_view fmt, Args&&... args) { std::cout << "[ERR] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl; }

    template <typename... Args>
    static inline void log_critical(std::string_view fmt, Args&&... args) { std::cout << "[TERMINATE] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
        __debugbreak();
    }
}