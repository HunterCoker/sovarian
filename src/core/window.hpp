#pragma once

#include "util/singleton.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <cstdint>

#include <iostream>


#define g_window    window::get()

typedef uint32_t    window_flags_t;

class window : public util::singleton {
public:
    window(const window&) = delete;
    void operator=(const window&) = delete;
    ~window();

    static window* get() {
        if (!_S_instance)
            _S_instance = new window;
        return _S_instance;
    }

    bool initialize() override;
    void present() const;
private:
    enum flags : flags_t {
        WINDOWED        = 0x00000001,
        FULLSCREEN      = 0x00000002,
        MINIMIZED       = 0x00000004,
        VSYNC           = 0x00000008,
    };
private:
    window()
        { _M_flags = WINDOWED | VSYNC; }
    static window* _S_instance;

    GLFWwindow* _M_window;
};
