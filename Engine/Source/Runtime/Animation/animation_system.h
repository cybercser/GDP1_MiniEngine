#pragma once

#include "Core/transform.h"

#include "animation_data.h"

namespace gdp1 {

struct AnimationClipRuntimeData {
    std::unordered_map<Transform*, std::vector<PositionKeyframe*>> xformPositionKeyframesMap;
    std::unordered_map<Transform*, std::vector<RotationKeyframe*>> xformRotationKeyframesMap;
    std::unordered_map<Transform*, std::vector<ScaleKeyframe*>> xformScaleKeyframesMap;
};

class AnimationSystem {
public:
    AnimationSystem(Animation* animation);
    ~AnimationSystem();

    void Update(float deltaTime);

    void PlayPreviousClip();
    void PlayNextClip();
    void SetPlaying(bool isPlaying);
    void SetPlaySpeed(float speed);

    float GetSpeed() const;
    size_t GetCurrentClipIndex() const;
    float GetElapsedTime() const;

private:
    void CreateRuntimeData();
    void SetCurrentClipIndex(size_t index);

private:
    Animation* m_Animation;  // original animation data

    // animation runtime data
    std::vector<AnimationClipRuntimeData> m_AnimationClipRuntimeData;

    size_t m_CurrentClipIndex = 0;
    float m_ElapsedTime = 0.0f;
    float m_PlaySpeed = 1.0f;

    bool m_Playing = false;
};

}  // namespace gdp1
