cmake_minimum_required(VERSION 3.26)
project(sovarian)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(GLAD_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
set(GLFW_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
set(GLM_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
set(TINYGLTF_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf)

add_subdirectory(${GLAD_DIR})
add_subdirectory(${GLFW_DIR})
add_subdirectory(${TINYGLTF_DIR})

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
    PUBLIC ${TINYGLTF_DIR}
)

target_link_libraries(sovarian
    PUBLIC glad
    PUBLIC glfw
    PUBLIC tinygltf
)
