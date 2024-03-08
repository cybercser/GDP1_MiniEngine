#include "model_bone.h"

namespace gdp1 {

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : name(name)
    , id(ID)
    , mLocalTransform(1.0f) {
    mNumPositionKeyFrames = channel->mNumPositionKeys;
    mPositionKeyFrames.resize(mNumPositionKeyFrames);

    for (int idxPosition = 0; idxPosition < mNumPositionKeyFrames; ++idxPosition) {
        const aiVector3D& aiPosition = channel->mPositionKeys[idxPosition].mValue;
        float timeStamp = (float)channel->mPositionKeys[idxPosition].mTime;
        convert::ToGLM(aiPosition, mPositionKeyFrames[idxPosition].value);
        mPositionKeyFrames[idxPosition].time = timeStamp;
    }

    mNumRotationKeyFrames = channel->mNumRotationKeys;
    mRotationKeyFrames.resize(mNumRotationKeyFrames);
    for (int idxRotation = 0; idxRotation < mNumRotationKeyFrames; ++idxRotation) {
        const aiQuaternion& aiOrientation = channel->mRotationKeys[idxRotation].mValue;
        float timeStamp = channel->mRotationKeys[idxRotation].mTime;
        convert::ToGLM(aiOrientation, mRotationKeyFrames[idxRotation].value);
        mRotationKeyFrames[idxRotation].time = timeStamp;
    }

    mNumScaleKeyFrames = channel->mNumScalingKeys;
    mScaleKeyFrames.resize(mNumScaleKeyFrames);
    for (int idxKey = 0; idxKey < mNumScaleKeyFrames; ++idxKey) {
        const aiVector3D& scale = channel->mScalingKeys[idxKey].mValue;
        float timeStamp = channel->mScalingKeys[idxKey].mTime;
        convert::ToGLM(scale, mScaleKeyFrames[idxKey].value);
        mScaleKeyFrames[idxKey].time = timeStamp;
    }
}

void Bone::Update(float animationTime) {
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    mLocalTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float animationTime) {
    for (int index = 0; index < mNumPositionKeyFrames - 1; ++index) {
        if (animationTime < mPositionKeyFrames[index + 1].time) return index;
    }
    return 0;
}

int Bone::GetRotationIndex(float animationTime) {
    for (int index = 0; index < mNumRotationKeyFrames - 1; ++index) {
        if (animationTime < mRotationKeyFrames[index + 1].time) return index;
    }
    return 0;
}

int Bone::GetScaleIndex(float animationTime) {
    for (int index = 0; index < mNumScaleKeyFrames - 1; ++index) {
        if (animationTime < mScaleKeyFrames[index + 1].time) return index;
    }
    return 0;
}

float Bone::GetScaleFactor(float lasttime, float nexttime, float animationTime) {
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lasttime;
    float framesDiff = nexttime - lasttime;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::InterpolatePosition(float animationTime) {
    if (1 == mNumPositionKeyFrames) return glm::translate(glm::mat4(1.0f), mPositionKeyFrames[0].value);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(mPositionKeyFrames[p0Index].time, mPositionKeyFrames[p1Index].time, animationTime);
    glm::vec3 finalPosition = glm::mix(mPositionKeyFrames[p0Index].value, mPositionKeyFrames[p1Index].value, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) {
    if (1 == mNumRotationKeyFrames) {
        auto rotation = glm::normalize(mRotationKeyFrames[0].value);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(mRotationKeyFrames[p0Index].time, mRotationKeyFrames[p1Index].time, animationTime);
    glm::quat finalRotation = glm::slerp(mRotationKeyFrames[p0Index].value, mRotationKeyFrames[p1Index].value, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float animationTime) {
    if (1 == mNumScaleKeyFrames) return glm::scale(glm::mat4(1.0f), mScaleKeyFrames[0].value);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(mScaleKeyFrames[p0Index].time, mScaleKeyFrames[p1Index].time, animationTime);
    glm::vec3 finalScale = glm::mix(mScaleKeyFrames[p0Index].value, mScaleKeyFrames[p1Index].value, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

}  // namespace gdp1