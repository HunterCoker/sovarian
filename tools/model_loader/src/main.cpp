#include <OpenGL/OpenGL.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "model_loader.hpp"

#include <memory>

void error_callback(int code, const char* message) {
    fprintf(stderr, "error (%d): %s\n", code, message);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct data {
    GLFWwindow* window;
    std::unique_ptr<shader> shader;
} _S_data;

int main(int argc, char* argv[]) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    _S_data.window = glfwCreateWindow(800, 600, "test window", nullptr, nullptr);
    if (!_S_data.window) {
        glfwTerminate();
        return 1;
    }
    glfwSetFramebufferSizeCallback(_S_data.window, framebuffer_size_callback);
    glfwMakeContextCurrent(_S_data.window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return 1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _S_data.shader
        = std::make_unique<shader>("../../../tools/model_loader/shader.glsl");

    model m("../../../assets/models/backpack/backpack.obj");

    while (!glfwWindowShouldClose(_S_data.window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        m.render(_S_data.shader);

        glfwPollEvents();
        glfwSwapBuffers(_S_data.window);
    }

    return 0;
}
