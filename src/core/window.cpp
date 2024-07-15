#include "core/window.hpp"

#include <cstdio>

void error_callback(int code, const char* message) {
    fprintf(stderr, "error (%d): %s\n", code, message);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

window* window::_S_instance = nullptr;

window::~window() {
    glfwDestroyWindow(_M_window);
    glfwTerminate();
}

bool window::initialize() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return false;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _M_window = glfwCreateWindow(800, 600, "sovarian", nullptr, nullptr);
    if (!_M_window) {
        glfwTerminate();
        return false;
    }
    glfwSetFramebufferSizeCallback(_M_window, framebuffer_size_callback);
    glfwMakeContextCurrent(_M_window);

    if (_M_flags | VSYNC)
        glfwSwapInterval(1);

    return true;
}

void window::present() const {
    glfwPollEvents();
    glfwSwapBuffers(_M_window); 
}