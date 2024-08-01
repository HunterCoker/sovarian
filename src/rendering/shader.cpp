#include "rendering/shader.hpp"

#include "util/log.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <string>

/* TODO: ensure this shader cannot be used if something fails in constructor */

shader::shader(const std::string_view shader_path) {
    std::ifstream shader_file;
	std::stringstream vertex_shader_stream;
    std::stringstream fragment_shader_stream;

    {
        enum shader_type { VERTEX, FRAGMENT, NONE };
        shader_type read_mode = shader_type::NONE;

        shader_file.open(shader_path.data());
        if (shader_file.fail()) {
            LOG_ERROR("could not open file [%s]", shader_path.data());
            return;
        }

        std::string line;
        uint32_t line_number = 0;
        while (std::getline(shader_file, line)) {
            line_number++;

            if (line.size() && line.front() == '/') {
                if      (line == "// !vertex")   read_mode = shader_type::VERTEX;
                else if (line == "// !fragment") read_mode = shader_type::FRAGMENT;
                else {
                    read_mode = shader_type::NONE;
                    LOG_ERROR("invalid shader type in shader file:\n\tsee here [%s]:"
                                "\n\t  |\n\t%d | %s\n\t  |  ^",
                        shader_path.data(), line_number, line.c_str());
                }
                continue;
            }

            switch (read_mode) {
                case shader_type::VERTEX: {
                    vertex_shader_stream << line << '\n';
                    break;
                }
                case shader_type::FRAGMENT: {
                    fragment_shader_stream << line << '\n';
                    break;
                }
                case shader_type::NONE: {
                    continue;
                }
            }
        }
    }

	std::string vertex_shader_str  = vertex_shader_stream.str();
	const char* vertex_shader_code = vertex_shader_str.c_str();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
	glCompileShader(vertex_shader);

	std::string fragment_shader_str  = fragment_shader_stream.str();
	const char* fragment_shader_code = fragment_shader_str.c_str();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);
	glCompileShader(fragment_shader);

	int  success;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		LOG_ERROR("vertex shader failed to compile!\n\tsee here [%s]:\n[\n%s]",
            shader_path.data(), info_log);
	}

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		LOG_ERROR("fragment shader failed to compile!\n\tsee here [%s]:\n[\n%s]",
            shader_path.data(), info_log);
	}

	_M_program = glCreateProgram();
	glAttachShader(_M_program, vertex_shader);
	glAttachShader(_M_program, fragment_shader);
	glLinkProgram(_M_program);

	glGetProgramiv(_M_program, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(_M_program, 512, NULL, info_log);
		LOG_ERROR("shader program failed to link!\n\tsee here [%s]:\n[\n%s]",
            shader_path.data(), info_log);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

shader::~shader() {
    glDeleteProgram(_M_program);
}

void shader::bind() const {
    glUseProgram(_M_program);
}

void shader::unbind() {
    glUseProgram(0);
}

void shader::set_uniform_1f(const std::string_view uniform, float value) const {
    GLint location = glGetUniformLocation(_M_program, uniform.data());
    glUniform1f(location, value);
}

void shader::set_uniform_3f(const std::string_view uniform, const glm::vec3& value) const {
    GLint location = glGetUniformLocation(_M_program, uniform.data());
    glUniform3f(location, value.x, value.y, value.z);
}

void shader::set_uniform_4f(const std::string_view uniform, const glm::vec4& value) const {
    GLint location = glGetUniformLocation(_M_program, uniform.data());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void shader::set_uniform_m4fv(const std::string_view uniform, const glm::mat4& value) const {
    GLint location = glGetUniformLocation(_M_program, uniform.data());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
