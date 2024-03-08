#pragma once

#include <Animation/animation_data.h>
#include <Render/model.h>
#include <string>

namespace gdp1 {
class Bone {

public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime);

    glm::mat4 GetLocalTransform() { return mLocalTransform; }
    
    std::string GetBoneName() const { return name; }
    
    int GetBoneID() { return id; }

    int GetPositionIndex(float animationTime);

    int GetRotationIndex(float animationTime);

    int GetScaleIndex(float animationTime);

private:
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 InterpolatePosition(float animationTime);

    glm::mat4 InterpolateRotation(float animationTime);

    glm::mat4 InterpolateScaling(float animationTime);

    std::vector<PositionKeyframe> mPositionKeyFrames;
    std::vector<RotationKeyframe> mRotationKeyFrames;
    std::vector<ScaleKeyframe> mScaleKeyFrames;

    int mNumPositionKeyFrames;
    int mNumRotationKeyFrames;
    int mNumScaleKeyFrames;

    glm::mat4 mLocalTransform;
    std::string name;
    int id;
};
}
