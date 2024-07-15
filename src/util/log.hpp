#pragma once

#include <string>
#include <cstdio>

#define LOG_INFO(message, ...) log_info(message, ##__VA_ARGS__)
template<typename... Args>
void log_info(const char* message, Args... args) {
    std::string message_str = std::string("info: ") + message + '\n';
    fprintf(stderr, message_str.c_str(), args...);
}

#define LOG_WARNING(message, ...) log_warning(message, ##__VA_ARGS__)
template<typename... Args>
void log_warning(const char* message, Args... args) {
    std::string message_str = std::string("warning: ") + message + '\n';
    fprintf(stderr, message_str.c_str(), args...);
}

#define LOG_ERROR(message, ...) log_error(message, ##__VA_ARGS__)
template<typename... Args>
void log_error(const char* message, Args... args) {
    std::string message_str = std::string("error: ") + message + '\n';
    fprintf(stderr, message_str.c_str(), args...);
}