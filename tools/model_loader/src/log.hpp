#pragma once

#include <cstdio>

#define LOG_INFO(message, ...) fprintf(stdout, "info: " message "\n", ##__VA_ARGS__)
#define LOG_WARNING(message, ...) fprintf(stderr, "warning: " message "\n", ##__VA_ARGS__)
#define LOG_ERROR(message, ...) fprintf(stderr, "error: " message "\n", ##__VA_ARGS__)
