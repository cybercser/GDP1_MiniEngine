#pragma once

#include <common.h>

#include "Physics/bounds.h"
#include "shader.h"

#include "assimp/Importer.hpp"
#include "Buffers/vao.h"
#include "Buffers/vbo.h"
#include "Buffers/ebo.h"
#include "Resource/texture.h"

#include <Core/cs_runner.h>

namespace gdp1 {

struct BoneMatrix {
    aiMatrix4x4 offset_matrix;
    aiMatrix4x4 final_world_transform;
};

class Mesh {
public:
    Mesh() = default;

    // mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo*> textures;

    Bounds bounds;

    bool isDynamicBuffer;

    // constructor
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int> indices, std::vector<TextureInfo*> textures,
         const Bounds& bounds, bool isDynamicBuffer = false);

    // render the mesh
    void DrawTextures(Shader* shader);
    void Draw(Shader* shader);

    // render the mesh
    void UpdateVertexBuffers();

    void SetupInstancing(std::vector<glm::mat4>& instanceMatrix, bool reset);

    void ResetInstancing();

    void DrawDebug(Shader* shader);

    void Setup();

private:
    // initializes all the buffer objects/arrays
    void SetupMesh();

    void SetupDebugData();

private:
    // render data
    VAO _VAO;
    VBO _VBO, _instanceVBO;

    // debug data
    VAO debugVAO;
    VBO debugVBO;
    std::vector<glm::vec3> boundsVertices;
    std::vector<unsigned int> boundsIndices;

    unsigned int numInstances = 1;
};

}  // namespace gdp1
