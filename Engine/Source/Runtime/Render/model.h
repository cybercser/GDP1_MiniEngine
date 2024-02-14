#pragma once

#include "mesh.h"
#include "shader.h"

#include <string>
#include <vector>

// #include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace gdp1 {

class Model {
public:
    // model data
    std::vector<TextureInfo>
        textures_loaded;  // stores all the textures loaded so far, optimization to make sure textures
    // aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    std::string shaderName;
    Bounds bounds;

    // constructor, expects a filepath to a 3D model.
    Model(const std::string& path, const std::string& shaderName, bool gamma = false);

    // draws the model, and thus all its meshes
    void Draw(Shader* shader);

    void DrawDebug(Shader* shader);

    unsigned int GetVertexCount() const;
    unsigned int GetTriangleCount() const;

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void LoadModel(std::string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this
    // process on its children nodes (if any).
    void ProcessNode(aiNode* node, const aiScene* scene);

    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<TextureInfo> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

private:
    unsigned int num_vertices_;
    unsigned int num_triangles_;
};

}  // namespace gdp1
