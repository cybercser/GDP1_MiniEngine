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
     
class CharacterAnimation;
class Animator;
class Model;

class Model {
public:
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

	void UpdateCharacterAnimation(float deltaTime);

    bool SetCurrentAnimation(const std::string& animationName);

    bool AddAnimation(const std::string& animationName, CharacterAnimation* animation);

    unsigned int GetVertexCount() const;
    unsigned int GetTriangleCount() const;

    unsigned int GetAnimationsCount() const;

    std::map<std::string, CharacterAnimation*> character_animations;

    std::map<std::string, sBoneInfo>& GetBoneInfoMap();
    int& GetBoneCount();

    glm::mat4 global_itranform;

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void LoadModel(std::string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this
    // process on its children nodes (if any).
    void ProcessNode(aiNode* node, const aiScene* scene);

    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void ProcessMeshBones(size_t baseVertex, aiMesh* mesh, const aiScene* scene);

    int bone_counter = 0;

    std::vector<sBoneInfo> boneMatrices;
    std::map<std::string, sBoneInfo> boneInfoMap;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<TextureInfo> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

private:
    unsigned int num_vertices_;
    unsigned int num_triangles_;

    size_t baseVertex = 0;
    size_t baseIndex = 0;

    GLuint boneLocations[MAX_BONES];

    Animator* animator;
};

namespace convert {

inline void ToGLM(const aiVector2D& input, glm::vec2& output) {
    output.x = input.x;
    output.y = input.y;
}
inline void ToGLM(const aiVector3D& input, glm::vec2& output) {
    output.x = input.x;
    output.y = input.y;
}
inline void ToGLM(const aiVector3D& input, glm::vec3& output) {
    output.x = input.x;
    output.y = input.y;
    output.z = input.z;
}
inline void ToGLM(const aiMatrix4x4& input, glm::mat4& output) {
    // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    output[0][0] = input.a1;
    output[1][0] = input.a2;
    output[2][0] = input.a3;
    output[3][0] = input.a4;
    output[0][1] = input.b1;
    output[1][1] = input.b2;
    output[2][1] = input.b3;
    output[3][1] = input.b4;
    output[0][2] = input.c1;
    output[1][2] = input.c2;
    output[2][2] = input.c3;
    output[3][2] = input.c4;
    output[0][3] = input.d1;
    output[1][3] = input.d2;
    output[2][3] = input.d3;
    output[3][3] = input.d4;
}
inline void ToGLM(const aiQuaternion& input, glm::quat& output) {
    output.w = input.w;
    output.x = input.x;
    output.y = input.y;
    output.z = input.z;
}

}  // namespace convert

}  // namespace gdp1
