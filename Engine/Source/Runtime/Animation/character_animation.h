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

struct LocalTransform {
    aiVector3D Scaling;
    aiQuaternion Rotation;
    aiVector3D Translation;
};

struct NodeInfo {
    NodeInfo() {}

    NodeInfo(const aiNode* n) { pNode = n; }

    const aiNode* pNode = NULL;
    bool isRequired = false;
};

class CharacterAnimation {
public:
    CharacterAnimation(const std::string& file, const std::string& name, Model* model);
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
    void calcInterpolatedPosition(aiVector3D& out, float p_animation_time, const aiNodeAnim* p_node_anim);
    void calcInterpolatedRotation(aiQuaternion& out, float p_animation_time, const aiNodeAnim* p_node_anim);
    void calcInterpolatedScaling(aiVector3D& out, float p_animation_time, const aiNodeAnim* p_node_anim);

    void readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform);
    void boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms);

    void boneTransformsBlended(float TimeInSeconds, std::vector<aiMatrix4x4>& blendedTransforms,
                               unsigned int startAnimIndex, unsigned int endAnimIndex, float blendFactor);

    void readNodeHierarchyBlended(float startAnimationTimeTicks, float endAnimationTimeTicks, const aiNode* pNode,
                                  const aiMatrix4x4& parentTransform, const aiAnimation& startAnimation,
                                  const aiAnimation& endAnimation, float blendFactor);

    float calcAnimationTimeTicks(float timeInSeconds, unsigned int animationIndex);
    void calcLocalTransform(LocalTransform& transform, float animationTimeTicks, const aiNodeAnim* nodeAnim);

    glm::mat4 aiToGlm(aiMatrix4x4 ai_matr);
    aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend);

    std::map<std::string, NodeInfo> m_requiredNodeMap;

    Model* model;

    CharacterAnimation* getAnimationByIndex(std::map<std::string, CharacterAnimation*>& animations, int index);

};

}  // namespace gdp1
