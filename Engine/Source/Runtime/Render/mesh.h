#pragma once

#include <common.h>

#include "Physics/bounds.h"
#include "shader.h"

#include "assimp\Importer.hpp"
#include "vao.h"
#include "vbo.h"
#include "ebo.h"

namespace gdp1 {

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

    unsigned int instancing;
    std::vector<glm::mat4> instanceMatrix;

    unsigned int VAO;
    Bounds bounds;

    bool isDynamicBuffer;

    // constructor
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures,
         unsigned int instancing, std::vector<glm::mat4> instanceMatrix,
         const Bounds& bounds, bool isDynamicBuffer = false);

    // render the mesh
    void Draw(Shader* shader, int numOfInstances);

    // render the mesh
    void UpdateVertexBuffers();

    void SetupInstancing();

    void DrawDebug(Shader* shader);

private:
    // initializes all the buffer objects/arrays
    void SetupMesh();

    void SetupDebugData();

private:
    // render data
    unsigned int VBO, EBO;
    unsigned int instanceVBO;

    // debug data
    unsigned int debugVAO, debugVBO, debugEBO;
    std::vector<glm::vec3> boundsVertices;
    std::vector<unsigned int> boundsIndices;
};

}  // namespace gdp1
