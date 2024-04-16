#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace gdp1 {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // bone indexes which will influence this vertex
    int boneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float weights[MAX_BONE_INFLUENCE];

    void SetBoneDefaults();
    void AddBoneData(unsigned int bone_id, float weight);
};

class VBO {
public:
    // Reference ID of the Vertex Buffer Object
    GLuint ID;

    // Constructor that generates a Vertex Buffer Object and links it to vertices
    VBO();

    void Generate();

    void BindData(std::vector<Vertex>& vertices, bool isDynamicBuffer);
    void BindData(std::vector<glm::mat4>& mat4s, bool isDynamicBuffer);
    void BindData(std::vector<glm::vec3>& vertices, bool isDynamicBuffer);

    // Update Buffers
    void UpdateVertexBuffers();

    // Binds the VBO
    void Bind();
    // Unbinds the VBO
    void Unbind();
    // Deletes the VBO
    void Delete();

private:
    std::vector<Vertex> vertices;
};

}  // namespace gdp1
