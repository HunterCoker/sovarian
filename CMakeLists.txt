cmake_minimum_required(VERSION 3.26)
project(sovarian)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(VENDOR_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
set(GLAD_DIR    ${VENDOR_DIR}/glad)
set(GLFW_DIR    ${VENDOR_DIR}/glfw)
set(GLM_DIR     ${VENDOR_DIR}/glm)

add_subdirectory(${GLAD_DIR})

set(GLFW_BUILD_STATIC_LIBS ON)
add_subdirectory(${GLFW_DIR})

option( SVRN_BUILD_TOOLS
    "build all tools used for developing sovarian"
    ON
)

IF(SVRN_BUILD_TOOLS)
    add_subdirectory(tools/model_loader)
ENDIF()

add_executable(sovarian
    src/main.cpp
    src/rendering/renderer.cpp
    src/rendering/shader.cpp
    src/core/window.cpp
)

target_include_directories(sovarian
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${GLAD_DIR}/include
    PUBLIC ${GLFW_DIR}/include
    PUBLIC ${GLM_DIR}
)

target_link_libraries(sovarian
    PUBLIC glad
    PUBLIC glfw
)
