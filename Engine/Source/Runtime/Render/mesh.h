#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Physics/bounds.h"
#include "shader.h"

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
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures,
         const Bounds& bounds);

    // render the mesh
    void Draw(std::shared_ptr<Shader> shader);

    void DrawDebug(std::shared_ptr<Shader> shader);

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
