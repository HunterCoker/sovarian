#include "rendering/renderer.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "util/log.hpp"
#include "rendering/shader.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

renderer* renderer::_S_instance = nullptr;

struct renderer_data {
    /* rendering data */
    std::unique_ptr<shader> pbr_shader;
    std::pair<GLuint, std::map<size_t, GLuint>> vao_and_ebos;
    //        ^ vao            ^ ?     ^ ebo
    /* glTF model data */
    /* uniform data */
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 light_pos;
};

static renderer_data _S_data;

static void bind_mesh(std::map<size_t, GLuint>& vbos,
                      const tinygltf::Model& model,
                      const tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const auto& buffer_view = model.bufferViews[i];

        // unsupported buffer view
        if (buffer_view.target == GL_NONE)
            continue;

        const auto& buffer = model.buffers[buffer_view.buffer];

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(buffer_view.target, vbo);
        glBufferData(buffer_view.target,
                    buffer_view.byteLength,
                    &buffer.data.at(0) + buffer_view.byteOffset,
                    GL_STATIC_DRAW);

        vbos[i] = vbo;
    }

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        const auto& primitive = mesh.primitives[i];
        const auto& index_accessor = model.accessors[primitive.indices];

        for (auto& attribute : primitive.attributes) {
            const auto& accessor = model.accessors[attribute.second];

            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = accessor.type != TINYGLTF_TYPE_SCALAR ? accessor.type : 1;
            int byte_stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

            int index = -1;
            if (attribute.first == "POSITION")   index = 0;
            if (attribute.first == "NORMAL")     index = 1;
            if (attribute.first == "TEXCOORD_0") index = 2;

            if (index != -1) {
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(index,
                                      size,
                                      accessor.componentType,
                                      accessor.normalized ? GL_TRUE : GL_FALSE,
                                      byte_stride,
                                      (void*)accessor.byteOffset);
            }
            else LOG_INFO("attrib array index missing (%d)", index);
        }

        if (model.textures.size() > 0) {
            // fixme: Use material's baseColor
            const auto& tex = model.textures[0];

            if (tex.source > -1) {
                GLuint texid;
                glGenTextures(1, &texid);

                const auto& image = model.images[tex.source];

                glBindTexture(GL_TEXTURE_2D, texid);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                GLenum format;
                switch (image.component) {
                    case 1:  { format = GL_RED; break; }
                    case 2:  { format = GL_RG;  break; }
                    case 3:  { format = GL_RGB; break; }
                    default: { format = GL_RGBA; }
                }

                GLenum type;
                switch (image.bits) {
                    case  8: { type = GL_UNSIGNED_BYTE;  break; }
                    case 16: { type = GL_UNSIGNED_SHORT; break; }
                    default: { }
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                            format, type, &image.image.at(0));
            }
        }
    }
}

/**
 * uses recursion to call 'bind_mesh' onto all scene nodes and its descendents
 */
static void bind_model_nodes(std::map<size_t, GLuint>& vbos,
                             const tinygltf::Model& model,
                             const tinygltf::Node& node) {
    if ((node.mesh != 0) && (node.mesh < model.meshes.size()))
        bind_mesh(vbos, model, model.meshes[node.mesh]);

    for (size_t i = 0; i < node.children.size(); ++i)
        bind_model_nodes(vbos, model, model.nodes[node.children[i]]);
};

/**
 * generates a vertex array with glTF model vertex data bound to GL_ARRAY_BUFFER as well as
 * generates element buffer objects for rendering of the data
 */
static std::pair<GLuint, std::map<size_t, GLuint>> bind_model(const tinygltf::Model& model) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const auto& scene = model.scenes[model.defaultScene];
    std::map<size_t, GLuint> vbos;
    for (size_t i = 0; i < scene.nodes.size(); ++i)
        bind_model_nodes(vbos, model, model.nodes[scene.nodes[i]]);

    glBindVertexArray(0);

    // cleanup vbos but do not delete index buffers yet
    for (auto it = vbos.cbegin(); it != vbos.cend();) {
        auto& bufferView = model.bufferViews[it->first];
        if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {
            glDeleteBuffers(1, &vbos[it->first]);
            vbos.erase(it++);
        }
        else {
            ++it;
        }
    }

    return {vao, vbos};
}

bool renderer::initialize() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create shader for rendering physically based materials
    _S_data.pbr_shader
        = std::make_unique<shader>("../assets/shaders/pbr_shader.glsl");

    // load glTF model and prepare it for rendering
    auto& model = _S_data.head_model;

    tinygltf::TinyGLTF loader;
    std::string err, warn;
    const std::string filename = "../assets/models/adam_head/adamHead.gltf";
    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty())  LOG_WARNING("%s", warn.c_str());
    if (!err.empty())   LOG_ERROR("%s", err.c_str());
    if (!res)           LOG_ERROR("failed to load gltf");
    else                LOG_INFO("successfully loaded gltf");

    _S_data.vao_and_ebos = bind_model(model);

    // initialize uniform data
    _S_data.model = glm::scale(glm::mat4(1.0f), glm::vec3(35.0f));
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float aspect = static_cast<float>(width) /
                   static_cast<float>(height);
    _S_data.projection = glm::perspective(45.0f, aspect, 0.1f, 100.0f);
    _S_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f));
    _S_data.light_pos = glm::vec3(0.3f, 2.0f, -1.0f);

    return true;
}

static void draw_mesh(const std::map<size_t, GLuint>& vbos,
                      const tinygltf::Model& model,
                      const tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        const auto& primitive = mesh.primitives[i];
        const auto& index_accessor = model.accessors[primitive.indices];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(index_accessor.bufferView));

        glDrawElements(primitive.mode,
                       index_accessor.count,
                       index_accessor.componentType,
                       (void*)index_accessor.byteOffset);
    }
}

static void draw_model_nodes(const std::pair<GLuint, std::map<size_t, GLuint>>& vao_and_ebos,
                             const tinygltf::Model& model,
                             const tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size()))
        draw_mesh(vao_and_ebos.second, model, model.meshes[node.mesh]);

    for (size_t i = 0; i < node.children.size(); i++)
        draw_model_nodes(vao_and_ebos, model, model.nodes[node.children[i]]);
}

static void draw_model(const std::pair<GLuint, std::map<size_t, GLuint>>& vao_and_ebos,
                       const tinygltf::Model& model) {
    glBindVertexArray(_S_data.vao_and_ebos.first);

    const auto& scene = model.scenes[model.defaultScene];

    for (size_t i = 0; i < scene.nodes.size(); ++i)
        draw_model_nodes(vao_and_ebos, model, model.nodes[scene.nodes[i]]);

    glBindVertexArray(0);
}

void renderer::render() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    _S_data.model = glm::rotate(_S_data.model, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));

    _S_data.pbr_shader->bind();
    _S_data.pbr_shader->set_uniform_m4fv("u_Model", _S_data.model);
    _S_data.pbr_shader->set_uniform_m4fv("u_View", _S_data.view);
    _S_data.pbr_shader->set_uniform_m4fv("u_Projection", _S_data.projection);
    _S_data.pbr_shader->set_uniform_3f("u_LightPosition", _S_data.light_pos);

    draw_model(_S_data.vao_and_ebos, _S_data.head_model);
}

void renderer::begin_frame() {
    // begin timer
}

void renderer::end_frame() {
    // end timer and set _M_stats.ms to the elapsed time in milliseconds
}
