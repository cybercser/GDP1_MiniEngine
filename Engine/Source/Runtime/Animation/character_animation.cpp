#include "character_animation.h"

#include <iostream>

namespace gdp1 {

CharacterAnimation::CharacterAnimation(const aiScene* scene1, const std::string& file, const std::string& name, Model* model): scene(nullptr) { 

    this->model = model;

    this->scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                        aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)  // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    assert(scene && scene->mRootNode);
    mRootNode = scene->mRootNode;

    if (scene->HasAnimations()) {
        if (scene->mAnimations[0]->mTicksPerSecond != 0.0) {
            ticks_per_second = scene->mAnimations[0]->mTicksPerSecond;
            duration = scene->mAnimations[0]->mDuration;
        } else {
            ticks_per_second = 25.0f;
        }
    }
}

CharacterAnimation::~CharacterAnimation() {}

unsigned int CharacterAnimation::findPosition(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumPositionKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mPositionKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

unsigned int CharacterAnimation::findRotation(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumRotationKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mRotationKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

unsigned int CharacterAnimation::findScaling(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumScalingKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mScalingKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

aiVector3D CharacterAnimation::calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumPositionKeys == 1) {
        return p_node_anim->mPositionKeys[0].mValue;
    }

    unsigned int position_index = findPosition(p_animation_time, p_node_anim);
    unsigned int next_position_index = position_index + 1;
    assert(next_position_index < p_node_anim->mNumPositionKeys);
    float delta_time = (float)(p_node_anim->mPositionKeys[next_position_index].mTime -
                               p_node_anim->mPositionKeys[position_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mPositionKeys[position_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mPositionKeys[position_index].mValue;
    aiVector3D end = p_node_anim->mPositionKeys[next_position_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}

aiQuaternion CharacterAnimation::calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumRotationKeys == 1) {
        return p_node_anim->mRotationKeys[0].mValue;
    }

    unsigned int rotation_index = findRotation(p_animation_time, p_node_anim);
    unsigned int next_rotation_index = rotation_index + 1;
    assert(next_rotation_index < p_node_anim->mNumRotationKeys);
    float delta_time = (float)(p_node_anim->mRotationKeys[next_rotation_index].mTime -
                               p_node_anim->mRotationKeys[rotation_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiQuaternion start_quat = p_node_anim->mRotationKeys[rotation_index].mValue;
    aiQuaternion end_quat = p_node_anim->mRotationKeys[next_rotation_index].mValue;

    return nlerp(start_quat, end_quat, factor);
}

aiVector3D CharacterAnimation::calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumScalingKeys == 1) {
        return p_node_anim->mScalingKeys[0].mValue;
    }

    unsigned int scaling_index = findScaling(p_animation_time, p_node_anim);
    unsigned int next_scaling_index = scaling_index + 1;
    assert(next_scaling_index < p_node_anim->mNumScalingKeys);
    float delta_time =
        (float)(p_node_anim->mScalingKeys[next_scaling_index].mTime - p_node_anim->mScalingKeys[scaling_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mScalingKeys[scaling_index].mValue;
    aiVector3D end = p_node_anim->mScalingKeys[next_scaling_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}

const aiNodeAnim* CharacterAnimation::findNodeAnim(const aiAnimation* p_animation, const std::string p_node_name) {
    for (unsigned int i = 0; i < p_animation->mNumChannels; i++) {
        const aiNodeAnim* node_anim = p_animation->mChannels[i];
        if (std::string(node_anim->mNodeName.data) == p_node_name) {
            return node_anim;
        }
    }

    return nullptr;
}
// start from RootNode
void CharacterAnimation::readNodeHierarchy(float p_animation_time, const aiNode* p_node,
                                           const aiMatrix4x4 parent_transform) {
    std::string node_name(p_node->mName.data);

    const aiAnimation* animation = scene->mAnimations[0];
    aiMatrix4x4 node_transform = p_node->mTransformation;

    const aiNodeAnim* node_anim = findNodeAnim(animation, node_name);

    if (node_anim) {
        // scaling
        // aiVector3D scaling_vector = node_anim->mScalingKeys[2].mValue;
        aiVector3D scaling_vector = calcInterpolatedScaling(p_animation_time, node_anim);
        aiMatrix4x4 scaling_matr;
        aiMatrix4x4::Scaling(scaling_vector, scaling_matr);

        // rotation
        // aiQuaternion rotate_quat = node_anim->mRotationKeys[2].mValue;
        aiQuaternion rotate_quat = calcInterpolatedRotation(p_animation_time, node_anim);
        aiMatrix4x4 rotate_matr = aiMatrix4x4(rotate_quat.GetMatrix());

        // translation
        // aiVector3D translate_vector = node_anim->mPositionKeys[2].mValue;
        aiVector3D translate_vector = calcInterpolatedPosition(p_animation_time, node_anim);
        aiMatrix4x4 translate_matr;
        aiMatrix4x4::Translation(translate_vector, translate_matr);

        node_transform = translate_matr * rotate_matr * scaling_matr;
    }

    aiMatrix4x4 global_transform = parent_transform * node_transform;

    auto& m_bone_mapping = model->GetBoneMap();
    auto& m_bone_matrices = model->GetBoneMatrices();

    if (m_bone_mapping.find(node_name) != m_bone_mapping.end())  // true if node_name exist in bone_mapping
    {
        unsigned int bone_index = m_bone_mapping[node_name];
        m_bone_matrices[bone_index].final_world_transform =
             model->m_global_inverse_transform * global_transform * m_bone_matrices[bone_index].offset_matrix;
    }

    for (unsigned int i = 0; i < p_node->mNumChildren; i++) {
        readNodeHierarchy(p_animation_time, p_node->mChildren[i], global_transform);
    }
}

void CharacterAnimation::boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms) {
    aiMatrix4x4 identity_matrix;

    std::cout << this->ticks_per_second << std::endl;
    double time_in_ticks = time_in_sec * this->ticks_per_second;
    float animation_time = fmod(time_in_ticks, duration);

    readNodeHierarchy(animation_time, mRootNode, identity_matrix);

    unsigned int m_num_bones = model->GetNumBones();
    transforms.resize(m_num_bones);

    for (unsigned int i = 0; i < m_num_bones; i++) {
        transforms[i] = model->GetBoneMatrices()[i].final_world_transform;
    }
}

glm::mat4 CharacterAnimation::aiToGlm(aiMatrix4x4 ai_matr) {
    glm::mat4 result;
    result[0].x = ai_matr.a1;
    result[0].y = ai_matr.b1;
    result[0].z = ai_matr.c1;
    result[0].w = ai_matr.d1;

    result[1].x = ai_matr.a2;
    result[1].y = ai_matr.b2;
    result[1].z = ai_matr.c2;
    result[1].w = ai_matr.d2;

    result[2].x = ai_matr.a3;
    result[2].y = ai_matr.b3;
    result[2].z = ai_matr.c3;
    result[2].w = ai_matr.d3;

    result[3].x = ai_matr.a4;
    result[3].y = ai_matr.b4;
    result[3].z = ai_matr.c4;
    result[3].w = ai_matr.d4;

    return result;
}

aiQuaternion CharacterAnimation::nlerp(aiQuaternion a, aiQuaternion b, float blend) {
    // cout << a.w + a.x + a.y + a.z << endl;
    a.Normalize();
    b.Normalize();

    aiQuaternion result;
    float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    float one_minus_blend = 1.0f - blend;

    if (dot_product < 0.0f) {
        result.x = a.x * one_minus_blend + blend * -b.x;
        result.y = a.y * one_minus_blend + blend * -b.y;
        result.z = a.z * one_minus_blend + blend * -b.z;
        result.w = a.w * one_minus_blend + blend * -b.w;
    } else {
        result.x = a.x * one_minus_blend + blend * b.x;
        result.y = a.y * one_minus_blend + blend * b.y;
        result.z = a.z * one_minus_blend + blend * b.z;
        result.w = a.w * one_minus_blend + blend * b.w;
    }

    return result.Normalize();
}

};  // namespace gdp1