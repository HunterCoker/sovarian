cmake_minimum_required(VERSION 3.26)
project(model_loader)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(ASSIMP_DIR  ${VENDOR_DIR}/assimp)
set(STB_DIR     ${VENDOR_DIR}/stb)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${ASSIMP_DIR} build)

add_executable(model_loader
    src/main.cpp
    src/shader.cpp
)

target_include_directories(model_loader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${GLAD_DIR}/include
    PUBLIC ${GLFW_DIR}/include
    PUBLIC ${GLM_DIR}
    PUBLIC ${ASSIMP_DIR}/include
    PUBLIC ${STB_DIR}
)

target_link_libraries(model_loader
    PUBLIC glad
    PUBLIC glfw
    PUBLIC assimp
)
