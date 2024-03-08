#pragma once

#include "Render/model.h"

#include <string>
#include <vector>
#include <map>

// #include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace gdp1 {

class CharacterAnimation {

    public:

    static const unsigned int MAX_BONES = 100;

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

}
