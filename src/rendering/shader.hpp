#pragma once

#include <glad/glad.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <string_view>

class shader {
public:
    explicit shader(const std::string_view shader_path);
    ~shader();

    shader(const shader&) = delete;
    void operator=(const shader&) = delete;
public:
    void bind() const;
    static void unbind();

    void set_uniform_1f(const std::string_view uniform, float value) const;
    void set_uniform_3f(const std::string_view uniform, const glm::vec3& value) const;
    void set_uniform_4f(const std::string_view uniform, const glm::vec4& value) const;
    void set_uniform_m4fv(const std::string_view uniform, const glm::mat4& value) const;
private:
    GLuint _M_program;
};