#pragma once

#include "Render/model.h"

#include <string>
#include <vector>
#include <map>

// #include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace gdp1 {

class Model;

class CharacterAnimation {
public:
    CharacterAnimation(const aiScene* scene, const std::string& file, const std::string& name, Model* model);
    ~CharacterAnimation();

    Assimp::Importer importer;
    const aiScene* scene;

    static const unsigned int MAX_BONES = 100;

    float ticks_per_second = 0.0f;
    float duration = 0.0f;

    float elapsedTime = 0.0f;

    aiNode* mRootNode;

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

    Model* model;
};

}  // namespace gdp1
