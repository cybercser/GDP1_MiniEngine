#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Physics/bounds.h"
#include "shader.h"

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 150

namespace gdp1 {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // bone indexes which will influence this vertex
    unsigned int boneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float weights[MAX_BONE_INFLUENCE];

    void AddBoneData(unsigned int boneId, float weight);

    void SetBoneDefaults() {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            boneIDs[i] = -1;
            weights[i] = 0.0f;
        }
    }
};

struct sBoneInfo {
    int id = -1;                        // index of the bone
    glm::mat4 offset = glm::mat4(1.f);  // offset matrix transforms vertex from model space to bone space
    glm::mat4 finalWorldTransform;
};

struct TextureInfo {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;
    unsigned int VAO;
    Bounds bounds;

    // constructor
    Mesh(size_t baseVertex, size_t baseIndex, std::vector<Vertex> vertices, std::vector<unsigned int> indices,
         std::vector<TextureInfo> textures, const Bounds& bounds);

    // render the mesh
    void Draw(Shader* shader);

    void DrawDebug(Shader* shader);

private:
    // initializes all the buffer objects/arrays
    void SetupMesh();

    void SetupDebugData();

private:
    // render data
    unsigned int VBO, EBO;

    size_t baseVertex;
    size_t baseIndex;

    // debug data
    unsigned int debugVAO, debugVBO, debugEBO;
    std::vector<glm::vec3> boundsVertices;
    std::vector<unsigned int> boundsIndices;
};

}  // namespace gdp1
