#pragma once

#include "assimp/types.h"
#include "glm/trigonometric.hpp"
#include <cstddef>
#include <glad/glad.h>
#include "GLFW/glfw3.h" /* remove this when done */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "assimp/material.h"
#include "shader.hpp"
#include "log.hpp"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <memory>
#include <string_view>
#include <vector>
#include <cstdlib>
#include <cstring>

#define MAX_BONE_INFLUENCE  4

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;
	GLint   bone_ids[MAX_BONE_INFLUENCE];
	GLfloat weights[MAX_BONE_INFLUENCE];
};

struct texture {
    GLuint handle;
    const GLchar* uniform;
    const GLchar* path;
};

class mesh {
public:
    explicit mesh(std::vector<vertex>   vertices,
                  std::vector<GLuint>   indices,
                  std::vector<texture*> textures)
            :_M_vertices(vertices), _M_indices(indices), _M_textures(textures) {
        glGenVertexArrays(1, &_M_vao);
        glBindVertexArray(_M_vao);

        glGenBuffers(1, &_M_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _M_vbo);
        glBufferData(GL_ARRAY_BUFFER, _M_vertices.size() * sizeof(vertex), _M_vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &_M_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _M_indices.size() * sizeof(GLuint), _M_indices.data(), GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
        // vertex bitangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, bitangent));
		// bone ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT,            sizeof(vertex), (void*)offsetof(vertex, bone_ids));
		// weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, weights));

        glBindVertexArray(0);
    }

    void render(const std::unique_ptr<shader>& shader) const {
        shader->bind();

        // set each sampler2D uniform to its corresponding material texture
        for (size_t i = 0; i < _M_textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, _M_textures[i]->handle);
            shader->set_uniform_1i(_M_textures[i]->uniform, i);
        }

        /* spins the model around at 45 degrees per seconds */
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
        float aspect = static_cast<float>(width) /
                       static_cast<float>(height);
        glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::radians((float)glfwGetTime() * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 v = glm::lookAt(glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 p = glm::perspective(45.0f, aspect, 0.1f, 100.0f);
        shader->set_uniform_m4fv("u_Model",      m);
        shader->set_uniform_m4fv("u_View",       v);
        shader->set_uniform_m4fv("u_Projection", p);

        glBindVertexArray(_M_vao);
        glDrawElements(GL_TRIANGLES,
                       _M_indices.size(),
                       GL_UNSIGNED_INT,
                       /* indices */ 0);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
        shader::unbind();
    }
private:
    std::vector<vertex> _M_vertices;
    std::vector<GLuint> _M_indices;
    std::vector<texture*> _M_textures;
    GLuint _M_vao, _M_vbo, _M_ebo;
};

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static GLuint load_texture(const char* filepath) {
    GLuint texture;
    glGenTextures(1, &texture);

    int width, height, comp;
    stbi_set_flip_vertically_on_load(true);
    uint8_t* image_data = stbi_load(filepath, &width, &height, &comp, 0);
    if (!image_data) {
        LOG_ERROR("could not read texture file\n\tsee here [%s]", filepath);
        return 0;
    }

    GLenum format;
    switch (comp) {
        case 1: { format = GL_RED;  break; }
        case 2: { format = GL_RG;   break; }
        case 3: { format = GL_RGB;  break; }
        case 4: { format = GL_RGBA; break; }
        default: {
            LOG_ERROR("invalid number of color chennels\n\tsee here [%s]", filepath);
            return 0;
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, /* level */ 0, format, width, height, /* border */ 0,
                 format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image_data);
    return texture;
}

static inline constexpr const char* get_uniform(aiTextureType type) {
    switch(type) {
        case aiTextureType_DIFFUSE: { return "u_TextureDiffuse"; }
        case aiTextureType_SPECULAR: { return "u_TextureSpecular"; }
        case aiTextureType_NORMALS: { return "u_TextureNormals"; }
        case aiTextureType_HEIGHT: { return "u_TextureHeight"; }
        default: { return ""; }
    }
}

class model {
public:
    explicit model(const std::string_view filepath)
            :_M_filepath(filepath) {
        size_t pos = _M_filepath.find_last_of('/');
        _M_filepath.erase(pos, std::string::npos);

        unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.data(), flags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            LOG_ERROR("assimp could not read file\n\tsee here [%s]:\n[\n%s\n]",
                filepath.data(), importer.GetErrorString());
            return;
        }
        else LOG_INFO("successfully read [%s]", filepath.data());

        process_node(scene->mRootNode, scene);
    }

    void render(const std::unique_ptr<shader>& shader) const {
        for (const auto& mesh : _M_meshes)
            mesh.render(shader);
    }

    /**
     * outputs raw model data to a file for easy loading in main application
     */
    void serialize(const std::string_view filename) const {

    }
private:
    void process_node(aiNode *node, const aiScene *scene) {
        // process each mesh located at the current node
        for (size_t i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            _M_meshes.push_back(process_mesh(mesh, scene));
        }

        for (size_t i = 0; i < node->mNumChildren; ++i)
            process_node(node->mChildren[i], scene);
    }

    mesh process_mesh(aiMesh *mesh, const aiScene *scene) {
        // create mesh object to render
        std::vector<vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<texture*> textures;

        // process vertices
        for (size_t i = 0; i < mesh->mNumVertices; ++i) {
            vertex v;

            if (mesh->HasPositions()) {
                v.position.x = mesh->mVertices[i].x;
                v.position.y = mesh->mVertices[i].y;
                v.position.z = mesh->mVertices[i].z;
            }

            if (mesh->HasNormals()) {
                v.normal.x = mesh->mNormals[i].x;
                v.normal.y = mesh->mNormals[i].y;
                v.normal.z = mesh->mNormals[i].z;
            }

            if (mesh->mTextureCoords[0]) {
                // texture coords
                /* a vertex can contain up to 8 different texture coordinates, but I'm not using models
                   where a vertex can have multiple texture coordinates, so I simply use the first set */
                v.uv.x = mesh->mTextureCoords[0][i].x;
                v.uv.y = mesh->mTextureCoords[0][i].y;

                if (mesh->HasTangentsAndBitangents()) {
                    // tangent
                    v.tangent.x = mesh->mTangents[i].x;
                    v.tangent.y = mesh->mTangents[i].y;
                    v.tangent.z = mesh->mTangents[i].z;
                    // bitangent
                    v.bitangent.x = mesh->mBitangents[i].x;
                    v.bitangent.y = mesh->mBitangents[i].y;
                    v.bitangent.z = mesh->mBitangents[i].z;
                }
            }

            vertices.push_back(v);
        }

        // process indices
        for (size_t i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<texture*> diffuse_maps
            = load_material_textures(material, aiTextureType_DIFFUSE);
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        std::vector<texture*> specular_maps
            = load_material_textures(material, aiTextureType_SPECULAR);
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

        std::vector<texture*> normal_maps
            = load_material_textures(material, aiTextureType_NORMALS);
        textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

        std::vector<texture*> height_maps
            = load_material_textures(material, aiTextureType_HEIGHT);
        textures.insert(textures.end(), height_maps.begin(), height_maps.end());

        if (textures.empty())
            LOG_WARNING("no material textures were loaded for mesh");

        return ::mesh(vertices, indices, textures);
    }

    std::vector<texture*> load_material_textures(aiMaterial* material, aiTextureType type) {
        std::vector<texture*> material_textures;

        for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
            aiString texture_path;

            if (material->GetTexture(type, i, &texture_path) != aiReturn_SUCCESS) {
                LOG_WARNING("no texture of type '%s' at index '%zu'",
                    get_uniform(type), i);
                continue;
            }

            std::string complete_filepath
                = _M_filepath + '/' + texture_path.C_Str();

            bool loaded = false;
            for (auto& texture : _M_loaded_textures) {
                if (std::strcmp(texture.path, complete_filepath.c_str()) == 0) {
                    material_textures.push_back(&texture);
                    loaded = true;
                    break;
                }
            }

            if (!loaded) {
                texture& texture = _M_loaded_textures.emplace_back();
                texture.handle  = load_texture(complete_filepath.c_str());
                texture.uniform = get_uniform(type);
                texture.path    = complete_filepath.c_str();
                material_textures.push_back(&texture);

                LOG_INFO("successfully loaded texture\n[\ntype:\t\t%s\nlocation\t%s\n]",
                    get_uniform(type), complete_filepath.c_str());
            }
        }

        return material_textures;
    }
private:
    std::vector<mesh> _M_meshes;
    std::vector<texture> _M_loaded_textures;
    std::string _M_filepath;
};
