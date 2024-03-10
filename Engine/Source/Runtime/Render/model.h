#pragma once

#include "mesh.h"
#include "shader.h"
#include "Animation/character_animation.h"

#include <string>
#include <vector>
#include <map>

// #include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace gdp1 {

class CharacterAnimation;

class Model {
public:
    static const unsigned int MAX_BONES = 100;

    // model data
    std::vector<TextureInfo> textures_loaded;  // stores all the textures loaded so far, optimization to make sure textures
    
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

    void SetCurrentAnimation(std::string name);

    void AddCharacterAnimation(std::string animationName, std::string animationPath);

    std::map<std::string, unsigned int>& GetBoneMap();

    std::vector<BoneMatrix>& GetBoneMatrices();

    unsigned int GetNumBones();

    aiMatrix4x4 m_global_inverse_transform;

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

    std::map<std::string, CharacterAnimation*> character_animations;

    CharacterAnimation* currentAnimation;

    std::map<std::string, unsigned int> m_bone_mapping;
    unsigned int m_num_bones = 0;
    std::vector<BoneMatrix> m_bone_matrices;

    double elapsedTime = 0.0f;

    Assimp::Importer importer;
    const aiScene* scene;
};

}  // namespace gdp1
