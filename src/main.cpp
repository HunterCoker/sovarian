#include "rendering/renderer.hpp"
#include "core/window.hpp"

#include "util/log.hpp"

#include <thread>

struct state {
    bool running;
} *g_state;

void renderer_command() {
    /*
     * window must be initialized here to make the openGL context
     * current on the rendering thread
     */ 
    if (g_window == nullptr || !g_window->initialize()) {
        LOG_ERROR("failed to initialize main window");
        std::exit(1);
    }

    if (g_renderer == nullptr || !g_renderer->initialize()) {
        LOG_ERROR("failed to initialize renderer");
        std::exit(1);
    }
    g_renderer->apply_flags(renderer::flags::ALL);

    while (g_state->running) { 
        g_renderer->begin_frame();
        g_renderer->render();
        g_renderer->end_frame();
        g_window->present();
    }
}

int main() {
    g_state = new state;
    g_state->running = true;

    std::thread render_thread(renderer_command);

    while (g_state->running) { 
        // gather statistics and feed them to debug object
        // auto& renderer_stats = g_renderer->get_stats();

    }
}