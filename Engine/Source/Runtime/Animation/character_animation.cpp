#include "character_animation.h"

#include <iostream>

namespace gdp1 {

CharacterAnimation::CharacterAnimation(const std::string& file, const std::string& name, Model* model)
    : scene(nullptr), mRootNode(NULL) {

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

    return 0;
}

unsigned int CharacterAnimation::findRotation(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumRotationKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mRotationKeys[i + 1].mTime) return i;
    }

    return 0;
}

unsigned int CharacterAnimation::findScaling(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumScalingKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mScalingKeys[i + 1].mTime) return i;
    }

    return 0;
}

void CharacterAnimation::calcInterpolatedPosition(aiVector3D& out, float animationTimeTicks,
                                                  const aiNodeAnim* pNodeAnim) {
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    unsigned int PositionIndex = findPosition(animationTimeTicks, pNodeAnim);
    unsigned int NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    if (t1 > animationTimeTicks) {
        out = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (animationTimeTicks - t1) / DeltaTime;
        // assert(Factor >= 0.0f && Factor <= 1.0f);
        if (Factor > 1.0f) Factor = 1.0f;
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        out = Start + Factor * Delta;
    }
}

void CharacterAnimation::calcInterpolatedRotation(aiQuaternion& out, float animationTimeTicks,
                                                  const aiNodeAnim* pNodeAnim) {
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    unsigned int RotationIndex = findRotation(animationTimeTicks, pNodeAnim);
    unsigned int NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    if (t1 > animationTimeTicks) {
        out = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (animationTimeTicks - t1) / DeltaTime;
        // assert(Factor >= 0.0f && Factor <= 1.0f);
        if (Factor > 1.0f) Factor = 1.0f;
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        aiQuaternion::Interpolate(out, StartRotationQ, EndRotationQ, Factor);
    }

    out.Normalize();
}

void CharacterAnimation::calcInterpolatedScaling(aiVector3D& out, float animationTimeTicks,
                                                 const aiNodeAnim* pNodeAnim) {
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1) {
        out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    unsigned int ScalingIndex = findScaling(animationTimeTicks, pNodeAnim);
    unsigned int NextScalingIndex = ScalingIndex + 1;
    if (NextScalingIndex < pNodeAnim->mNumScalingKeys && ScalingIndex < pNodeAnim->mNumScalingKeys) {
        float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
        if (t1 > animationTimeTicks) {
            out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        } else {
            float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
            float DeltaTime = t2 - t1;
            float Factor = (animationTimeTicks - (float)t1) / DeltaTime;
            if (Factor > 1.0f) Factor = 1.0f;
            // assert(Factor >= 0.0f && Factor <= 1.0f);
            const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
            const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
            aiVector3D Delta = End - Start;
            out = Start + Factor * Delta;
        }
    } else {
        // Handle the case where NextScalingIndex exceeds the maximum index
        out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    }
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
        LocalTransform transform;
        calcLocalTransform(transform, p_animation_time, node_anim);

        aiMatrix4x4 scaling_matr;
        aiMatrix4x4::Scaling(transform.Scaling, scaling_matr);

        aiMatrix4x4 rotate_matr = aiMatrix4x4(transform.Rotation.GetMatrix());

        aiMatrix4x4 translate_matr;
        aiMatrix4x4::Translation(transform.Translation, translate_matr);

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

    double time_in_ticks = time_in_sec * this->ticks_per_second;
    float animation_time = fmod(time_in_ticks, duration);

    readNodeHierarchy(animation_time, mRootNode, identity_matrix);

    unsigned int m_num_bones = model->GetNumBones();
    transforms.resize(m_num_bones);

    for (unsigned int i = 0; i < m_num_bones; i++) {
        transforms[i] = model->GetBoneMatrices()[i].final_world_transform;
    }
}

void CharacterAnimation::boneTransformsBlended(float TimeInSeconds, std::vector<aiMatrix4x4>& blendedTransforms,
                                               unsigned int startAnimIndex, unsigned int endAnimIndex,
                                               float blendFactor) {
    /*if (startAnimIndex >= scene->mNumAnimations) {
        printf("Invalid start animation index %d, max is %d\n", startAnimIndex, scene->mNumAnimations);
        assert(0);
    }

    if (endAnimIndex >= scene->mNumAnimations) {
        printf("Invalid end animation index %d, max is %d\n", endAnimIndex, scene->mNumAnimations);
        assert(0);
    }*/

    if ((blendFactor < 0.0f) || (blendFactor > 1.0f)) {
        printf("Invalid blend factor %f\n", blendFactor);
        assert(0);
    }

    float StartAnimationTimeTicks = calcAnimationTimeTicks(TimeInSeconds, startAnimIndex);
    float EndAnimationTimeTicks = calcAnimationTimeTicks(TimeInSeconds, endAnimIndex);

    // const aiAnimation& StartAnimation = *scene->mAnimations[startAnimIndex];
    // const aiAnimation& EndAnimation = *scene->mAnimations[endAnimIndex];
    const aiAnimation& StartAnimation =
        *(getAnimationByIndex(model->character_animations, startAnimIndex)->scene->mAnimations[0]);
    const aiAnimation& EndAnimation =
        *(getAnimationByIndex(model->character_animations, endAnimIndex)->scene->mAnimations[0]);

    aiMatrix4x4 Identity;
    readNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, scene->mRootNode, Identity, StartAnimation,
                             EndAnimation, blendFactor);

    auto& m_bone_mapping = model->GetBoneMap();
    auto& m_bone_matrices = model->GetBoneMatrices();

    blendedTransforms.resize(m_bone_mapping.size());

    for (unsigned int i = 0; i < m_bone_matrices.size(); i++) {
        blendedTransforms[i] = m_bone_matrices[i].final_world_transform;
    }
}

void CharacterAnimation::readNodeHierarchyBlended(float startAnimationTimeTicks, float endAnimationTimeTicks,
                                                  const aiNode* pNode, const aiMatrix4x4& parentTransform,
                                                  const aiAnimation& startAnimation, const aiAnimation& endAnimation,
                                                  float blendFactor) {
    std::string NodeName(pNode->mName.data);

    aiMatrix4x4 NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pStartNodeAnim = findNodeAnim(&startAnimation, NodeName);

    LocalTransform startTransform;

    if (pStartNodeAnim) {
        calcLocalTransform(startTransform, startAnimationTimeTicks, pStartNodeAnim);
    }

    LocalTransform endTransform;

    const aiNodeAnim* pEndNodeAnim = findNodeAnim(&endAnimation, NodeName);

    if ((pStartNodeAnim && !pEndNodeAnim) || (!pStartNodeAnim && pEndNodeAnim)) {
        LOG_ERROR("On the node {0} there is an animation node for only one of the start/end animations.", NodeName.c_str());
        LOG_ERROR("This case is not supported");
        exit(0);
    }

    if (pEndNodeAnim) {
        calcLocalTransform(endTransform, endAnimationTimeTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && pEndNodeAnim) {
        // Interpolate scaling
        const aiVector3D& Scale0 = startTransform.Scaling;
        const aiVector3D& Scale1 = endTransform.Scaling;

        aiVector3D BlendedScaling = (1.0f - blendFactor) * Scale0 + Scale1 * blendFactor;
        aiMatrix4x4 ScalingM;
        aiMatrix4x4::Scaling(BlendedScaling, ScalingM);

        // Interpolate rotation
        const aiQuaternion& Rot0 = startTransform.Rotation;
        const aiQuaternion& Rot1 = endTransform.Rotation;
        aiQuaternion BlendedRot;
        aiQuaternion::Interpolate(BlendedRot, Rot0, Rot1, blendFactor);
        aiMatrix4x4 RotationM = aiMatrix4x4(BlendedRot.GetMatrix());

        // Interpolate translation
        const aiVector3D& Pos0 = startTransform.Translation;
        const aiVector3D& Pos1 = endTransform.Translation;
        aiVector3D BlendedTranslation = (1.0f - blendFactor) * Pos0 + Pos1 * blendFactor;

        aiMatrix4x4 TranslationM;
        aiMatrix4x4::Translation(BlendedTranslation, TranslationM);

        // Combine it all
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    aiMatrix4x4 GlobalTransformation = parentTransform * NodeTransformation;

    auto& m_bone_mapping = model->GetBoneMap();
    auto& m_bone_matrices = model->GetBoneMatrices();

    if (m_bone_mapping.find(NodeName) != m_bone_mapping.end()) {
        unsigned int BoneIndex = m_bone_mapping[NodeName];
        m_bone_matrices[BoneIndex].final_world_transform =
            model->m_global_inverse_transform * GlobalTransformation * m_bone_matrices[BoneIndex].offset_matrix;
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        /*std::string ChildName(pNode->mChildren[i]->mName.data);

        std::map<std::string, NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            readNodeHierarchyBlended(startAnimationTimeTicks, endAnimationTimeTicks, pNode->mChildren[i],
                                     GlobalTransformation, startAnimation, endAnimation, blendFactor);
        }*/

        readNodeHierarchyBlended(startAnimationTimeTicks, endAnimationTimeTicks, pNode->mChildren[i],
                                 GlobalTransformation, startAnimation, endAnimation, blendFactor);
    }
}

float CharacterAnimation::calcAnimationTimeTicks(float timeInSeconds, unsigned int animationIndex) {
    CharacterAnimation* anim = getAnimationByIndex(model->character_animations, animationIndex);
    float TicksPerSecond = (float)(anim->ticks_per_second != 0 ? anim->ticks_per_second : 25.0f);
    float TimeInTicks = timeInSeconds * TicksPerSecond;
    // we need to use the integral part of mDuration for the total length of the animation
    float Duration = 0.0f;
    // float fraction = modf((float)scene->mAnimations[animationIndex]->mDuration, &Duration);
    float fraction = modf((float)anim->duration, &Duration);
    float AnimationTimeTicks = fmod(TimeInTicks, Duration);
    return AnimationTimeTicks;
}

CharacterAnimation* CharacterAnimation::getAnimationByIndex(std::map<std::string, CharacterAnimation*>& animations,
                                                            int index) {
    auto it = animations.begin();
    std::advance(it, index);
    return it != animations.end() ? it->second : nullptr;
}

void CharacterAnimation::calcLocalTransform(LocalTransform& transform, float animationTimeTicks,
                                            const aiNodeAnim* nodeAnim) {
    calcInterpolatedScaling(transform.Scaling, animationTimeTicks, nodeAnim);
    calcInterpolatedRotation(transform.Rotation, animationTimeTicks, nodeAnim);
    calcInterpolatedPosition(transform.Translation, animationTimeTicks, nodeAnim);
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