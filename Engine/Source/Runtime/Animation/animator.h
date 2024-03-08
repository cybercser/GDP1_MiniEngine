#pragma once

#include "Animation/character_animation.h"

namespace gdp1 {

class Animator {

public:

    Animator(CharacterAnimation* animation);

    void UpdateAnimation(float dt);

    void PlayAnimation(CharacterAnimation* pAnimation);
    
    void CalculateBoneTransform(const Node* node, glm::mat4 parentTransform);

    std::vector<glm::mat4> GetFinalBoneMatrices();

    inline float GetElapsedTime() const { return mCurrentTime; }

private:
    glm::mat4 globalInverseTransform;
    std::vector<glm::mat4> mFinalBoneMatrices;
    CharacterAnimation* mCurrentAnimation;
    float mCurrentTime;
    float mDeltaTime;
};

}
