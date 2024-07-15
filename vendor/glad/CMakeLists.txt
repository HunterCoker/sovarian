cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)

project(glad VERSION 3.3 LANGUAGES C CXX)

add_library(glad STATIC
    glad.c
)

target_include_directories(glad
    PUBLIC include
)