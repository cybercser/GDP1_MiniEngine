#include "animator.h"

namespace gdp1 {

Animator::Animator(CharacterAnimation* animation) {
    mDeltaTime = 0.001f;
    mCurrentTime = 0.0;
    mCurrentAnimation = animation;

    mFinalBoneMatrices.reserve(150);

    for (int i = 0; i < 150; i++) mFinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt) {
    mDeltaTime = dt;
    if (mCurrentAnimation) {
        mCurrentTime += mCurrentAnimation->GetTicksPerSecond() * dt;
        mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->GetDuration());
        globalInverseTransform = glm::inverse(mCurrentAnimation->GetRootNode().transformation);
        CalculateBoneTransform(&mCurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
}

void Animator::PlayAnimation(CharacterAnimation* pAnimation) {
    mCurrentAnimation = pAnimation;
    mCurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const Node* node, glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = mCurrentAnimation->FindBone(nodeName);

    if (bone) {
        bone->Update(mCurrentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = mCurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        mFinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->children_count; i++) {
        CalculateBoneTransform(&node->children[i], globalTransformation);
    }
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices() { return mFinalBoneMatrices; }

}  // namespace gdp1