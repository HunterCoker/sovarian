#pragma once

#include "util/singleton.hpp"
#include <glad/glad.h>

#include <cstddef>

#define g_renderer  renderer::get()

class renderer : public util::singleton {
public:
    renderer(const renderer&) = delete;
    void operator=(const renderer&) = delete;
    ~renderer() = default;

    static renderer* get() {
        if (!_S_instance)
            _S_instance = new renderer;
        return _S_instance;
    }
    
    bool initialize() override;

    void render() const;
    void begin_frame();
    void end_frame();
public:
    // flags enable certain post processing effects to be applied
    enum flags : flags_t {
        BLOOM           = 0x00000001,
        SHADOWS         = 0x00000002,
        ANTIALIASING    = 0x00000004,
        ALL             = 0xffffffff,
    };
    // stats of the last rendered frame
    struct stats {
        size_t ms;
        size_t draw_calls;
    };
    const stats& get_stats() const { return _M_stats; }
private:
    renderer() = default;
    static renderer* _S_instance;
    
    ::renderer::stats _M_stats;
};