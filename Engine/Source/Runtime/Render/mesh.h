#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Physics/bounds.h"
#include "shader.h"

#include "assimp\Importer.hpp"

#define MAX_BONE_INFLUENCE 4

namespace gdp1 {

struct sVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

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

struct BoneMatrix {
    aiMatrix4x4 offset_matrix;
    aiMatrix4x4 final_world_transform;
};

struct TextureInfo {
    unsigned int id;
    std::string type;
    std::string path;
    bool hasFBO;
};

class Mesh {
public:
    Mesh() = default;

    // mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;
    unsigned int VAO;
    Bounds bounds;

    bool isDynamicBuffer;

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures,
         const Bounds& bounds, bool isDynamicBuffer = false);

    // render the mesh
    void Draw(Shader* shader);

    // render the mesh
    void UpdateVertexBuffers();

    void DrawDebug(Shader* shader);

private:
    // initializes all the buffer objects/arrays
    void SetupMesh();

    void SetupDebugData();

private:
    // render data
    unsigned int VBO, EBO;

    // debug data
    unsigned int debugVAO, debugVBO, debugEBO;
    std::vector<glm::vec3> boundsVertices;
    std::vector<unsigned int> boundsIndices;
};

}  // namespace gdp1
