#include "character_animation.h"

namespace gdp1 {

CharacterAnimation::CharacterAnimation(const std::string& file, Model* model, const std::string& name)
    : mName(name) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);

    auto animation = scene->mAnimations[0];
    mDuration = animation->mDuration;
    mTicksPerSecond = animation->mTicksPerSecond;

    aiMatrix4x4 gTransform = scene->mRootNode->mTransformation;
    convert::ToGLM(gTransform.Inverse(), model->global_itranform);

    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadBones(animation, *model);
}

Bone* CharacterAnimation::FindBone(const std::string& name) {
    auto iter =
        std::find_if(mBones.begin(), mBones.end(), [&](const Bone& Bone) { return Bone.GetBoneName() == name; });
    if (iter == mBones.end())
        return nullptr;
    else
        return &(*iter);
}

void CharacterAnimation::ReadBones(const aiAnimation* animation, Model& model) {
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();  // getting m_BoneInfoMap from Model class
    int& boneCount = model.GetBoneCount();       // getting the m_BoneCounter from Model class

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        mBones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

inline void CharacterAnimation::ReadHierarchyData(Node& dest, const aiNode* src) {
    assert(src);

    dest.name = src->mName.data;
    convert::ToGLM(src->mTransformation, dest.transformation);
    dest.children_count = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        Node newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
}  // namespace nGraphics
