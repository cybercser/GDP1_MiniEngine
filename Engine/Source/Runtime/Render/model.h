#pragma once

#include "mesh.h"
#include "shader.h"

#include <string>
#include <vector>
#include <map>

// #include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace gdp1 {

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

private:
    Assimp::Importer importer;
    const aiScene* scene;

    std::map<std::string, unsigned int> m_bone_mapping;  // maps a bone name and their index
    unsigned int m_num_bones = 0;
    std::vector<BoneMatrix> m_bone_matrices;
    aiMatrix4x4 m_global_inverse_transform;

    GLuint m_bone_location[MAX_BONES];
    float ticks_per_second = 0.0f;
    float duration = 0.0f;

    float elapsedTime = 0.0f;

    unsigned int findPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
    unsigned int findRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
    unsigned int findScaling(float p_animation_time, const aiNodeAnim* p_node_anim);
    const aiNodeAnim* findNodeAnim(const aiAnimation* p_animation, const std::string p_node_name);

    // calculate transform matrix
    aiVector3D calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
    aiQuaternion calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
    aiVector3D calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim);

    void readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform);
    void boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms);

    glm::mat4 aiToGlm(aiMatrix4x4 ai_matr);
    aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend);

};

}  // namespace gdp1
