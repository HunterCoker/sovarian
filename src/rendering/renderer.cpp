#include "rendering/renderer.hpp"

#include "util/log.hpp"
#include "rendering/shader.hpp"

#include <GLFW/glfw3.h>
#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

renderer* renderer::_S_instance = nullptr;

struct renderer_data {
    /* rendering data */
    std::unique_ptr<shader> pbr_shader;
    std::pair<GLuint, std::map<size_t, GLuint>> vao_and_ebos;
    //        ^ vao            ^ ?     ^ ebo
    /* glTF model data */
    tinygltf::Model head_model;
    /* uniform data */
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 light_pos;
};

static renderer_data _S_data;

bool renderer::initialize() { 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return false;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create shader for rendering physically based materials    
    _S_data.pbr_shader
        = std::make_unique<shader>("../../assets/shaders/pbr_shader.glsl");

    // load glTF model and prepare it for rendering
    auto& model = _S_data.head_model;

    tinygltf::TinyGLTF loader;
    std::string err, warn;
    const std::string filename = "../../assets/models/adam_head/adamHead.gltf";
    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty())  LOG_WARNING("%s", warn.c_str());
    if (!err.empty())   LOG_ERROR("%s", err.c_str());
    if (!res)           LOG_ERROR("failed to load gltf");
    else                LOG_INFO("successfully loaded gltf");
    
    // auto bind_mesh = [&](std::map<size_t, GLuint>& vbos,
    //                     const tinygltf::Model& model,
    //                     const tinygltf::Mesh& mesh) -> void {
    //     for (size_t i = 0; i < model.bufferViews.size(); ++i) {
    //         const auto& buffer_view = model.bufferViews[i];
    //         if (buffer_view.target == 0)
    //             continue;

    //         const auto& buffer = model.buffers[buffer_view.buffer];

    //         GLuint vbo;
    //         glGenBuffers(1, &vbo);
    //         glBindBuffer(buffer_view.target, vbo);
    //         glBufferData(buffer_view.target,
    //                      buffer_view.byteLength,
    //                      &buffer.data.at(0) + buffer_view.byteOffset,
    //                      GL_STATIC_DRAW);

    //         vbos[i] = vbo;
    //     }

    //     // for (;;) { ... }
    // };

    // // uses recursion to call 'bind_mesh' onto all scene nodes and its descendents
    // auto bind_model_nodes = [&](std::map<size_t, GLuint>& vbos,
    //                            const tinygltf::Model& model,
    //                            const tinygltf::Node& node) -> void {
    //     if ((node.mesh != 0) && (node.mesh < model.meshes.size()))
    //         bind_mesh(vbos, model, model.meshes[node.mesh]);

    //     for (size_t i = 0; i < node.children.size(); ++i)
    //         bind_model_nodes(vbos, model, model.nodes[node.children[i]]);
    // };

    // auto bind_model
    //         = [&](const tinygltf::Model& model) -> std::pair<GLuint, std::map<size_t, GLuint>> {
    //     GLuint vao;
    //     glGenVertexArrays(1, &vao);
    //     glBindVertexArray(vao);

    //     const auto& scene = model->scenes[model->defaultScene];
    //     std::map<size_t, GLuint> vbos;
    //     for (size_t i = 0; i < scene.nodes.size(); ++i)
    //         bind_model_nodes(vbos, model, model.nodes[scene.nodes[i]]);
    
    //     glBindVertexArray(0);

    //     // cleanup vbos but do not delete index buffers yet
    //     for (auto it = vbos.cbegin(); it != vbos.cend();) {
    //         auto& bufferView = model.bufferViews[it->first];
    //         if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {
    //             glDeleteBuffers(1, &vbos[it->first]);
    //             vbos.erase(it++);
    //         }
    //         else {
    //             ++it;
    //         }
    //     }

    //     return {vao, vbos};
    // }

    // _S_data.vao_and_ebos = bind_model();

    // initialize uniform data
    _S_data.model = glm::mat4(1.0f);
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float aspect = static_cast<float>(width) /
                   static_cast<float>(height);
    _S_data.projection = glm::perspective(45.0f, aspect, 0.01f, 100.0f);
    _S_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f));
    _S_data.light_pos = glm::vec3(0.3f, 2.0f, -1.0f);

    return true;
}

void renderer::render() const {
    _S_data.pbr_shader->bind();
    _S_data.model = glm::translate(glm::mat4(1.0f), glm::vec3(glfwGetTime())) *
                    glm::rotate(glm::mat4(1.0f),
                                glm::radians(static_cast<float>(glfwGetTime())),
                                glm::vec3(0.0f, 1.0f, 0.0f));
    
    _S_data.pbr_shader->set_uniform_m4fv("u_Model", _S_data  .model);
    _S_data.pbr_shader->set_uniform_m4fv("u_View", _S_data.view);
    _S_data.pbr_shader->set_uniform_m4fv("u_Projection", _S_data.projection);
    _S_data.pbr_shader->set_uniform_3f("u_LightPosition", _S_data.light_pos);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    // const auto& model = _S_data.model;
    // const auto& scene = model->scenes[model->defaultScene];

    // draw every node in the default scene recursively
    // auto draw_mesh = [](tinygltf::Model& model, tinygltf::Mesh& mesh) -> void {
    //     for (size_t i = 0; i < mesh.primitives.size(); ++i) {
    //         auto& primitive = mesh.primitives[i];
    //         auto& accessor  = model.accessors[primitive.indices];

    //         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[primitive.indices]);
    //         glDrawElements(primitive.mode,
    //                        accessor.count,
    //                        accessor.componentType,
    //                        (int8_t*)nullptr + accessor.byteOffset);
    //     }
    // }

    // auto draw_nodes = []() -> void {

    // }

    // for (size_t i = 0; i < scene.nodes.size(); ++i) {
        
    //     model.nodes[scene.nodes[i]]
    // }
}

void renderer::begin_frame() {
    // begin timer
}

void renderer::end_frame() {
    // end timer and set _M_stats.ms to the elapsed time in milliseconds
}
