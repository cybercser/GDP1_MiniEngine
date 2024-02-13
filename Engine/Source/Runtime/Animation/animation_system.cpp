#include "animation_system.h"

#include <glm/gtx/easing.hpp>

namespace gdp1 {

AnimationSystem::AnimationSystem(Animation* animation)
    : m_Animation(animation)
    , m_Playing(false)
    , m_CurrentClipIndex(0)
    , m_ElapsedTime(0.0)
    , m_PlaySpeed(1.0f) {
    CreateRuntimeData();
}

AnimationSystem::~AnimationSystem() {
    delete m_Animation;
}

void AnimationSystem::SetPlaying(bool isPlaying) {
    m_Playing = isPlaying;
}

void AnimationSystem::CreateRuntimeData() {
    // for each clip, sort the keyframes by time
    for (auto& clip : m_Animation->clips) {
        std::sort(clip.positionKeys.begin(), clip.positionKeys.end(),
                  [](const PositionKeyframe& a, const PositionKeyframe& b) { return a.time < b.time; });

        std::sort(clip.scaleKeys.begin(), clip.scaleKeys.end(),
                  [](const ScaleKeyframe& a, const ScaleKeyframe& b) { return a.time < b.time; });

        std::sort(clip.rotationKeys.begin(), clip.rotationKeys.end(),
                  [](const RotationKeyframe& a, const RotationKeyframe& b) { return a.time < b.time; });
    }
    // after sorting, gather the keyframes for each game object
    m_AnimationClipRuntimeData.clear();
    for (auto& clip : m_Animation->clips) {
        AnimationClipRuntimeData data;
        for (auto& keyframe : clip.positionKeys) {
            if (data.xformPositionKeyframesMap.find(keyframe.xform) == data.xformPositionKeyframesMap.end()) {
                data.xformPositionKeyframesMap[keyframe.xform] = std::vector<PositionKeyframe*>();
            }
            data.xformPositionKeyframesMap[keyframe.xform].push_back(&keyframe);
        }
        for (auto& keyframe : clip.scaleKeys) {
            if (data.xformScaleKeyframesMap.find(keyframe.xform) == data.xformScaleKeyframesMap.end()) {
                data.xformScaleKeyframesMap[keyframe.xform] = std::vector<ScaleKeyframe*>();
            }
            data.xformScaleKeyframesMap[keyframe.xform].push_back(&keyframe);
        }
        for (auto& keyframe : clip.rotationKeys) {
            if (data.xformRotationKeyframesMap.find(keyframe.xform) == data.xformRotationKeyframesMap.end()) {
                data.xformRotationKeyframesMap[keyframe.xform] = std::vector<RotationKeyframe*>();
            }
            data.xformRotationKeyframesMap[keyframe.xform].push_back(&keyframe);
        }
        m_AnimationClipRuntimeData.push_back(data);
    }
}

void AnimationSystem::SetCurrentClipIndex(size_t index) {
    m_CurrentClipIndex = index;
    m_ElapsedTime = 0;
}

void AnimationSystem::SetPlaySpeed(float speed) {
    m_PlaySpeed = speed;
}

float AnimationSystem::GetSpeed() const {
    return m_PlaySpeed;
}

size_t AnimationSystem::GetCurrentClipIndex() const {
    return m_CurrentClipIndex;
}

float AnimationSystem::GetElapsedTime() const {
    return m_ElapsedTime;
}

void AnimationSystem::Update(float deltaTime) {
    if (!m_Playing) {
        return;
    }

    m_ElapsedTime += deltaTime * m_PlaySpeed;

    AnimationClipRuntimeData& clip = m_AnimationClipRuntimeData[m_CurrentClipIndex];

    /// POSITION
    for (auto& xformKeysPair : clip.xformPositionKeyframesMap) {
        Transform* xform = xformKeysPair.first;
        std::vector<PositionKeyframe*>& keyframes = xformKeysPair.second;

        // If there is only 1 key frame, use that one.
        // If we are equal to or greater than the time of the last key frame, use the last keyframe
        if (keyframes.size() == 1) {
            xform->SetPosition(keyframes[0]->value);
        } else {
            // Find our position keyframes (Start and End)
            int KeyframeEndIndex = 0;
            for (; KeyframeEndIndex < keyframes.size(); KeyframeEndIndex++) {
                if (keyframes[KeyframeEndIndex]->time > m_ElapsedTime) {
                    break;
                }
            }

            if (KeyframeEndIndex >= keyframes.size()) {
                // we are at or past the last key frame use the last keyframe only
                xform->SetPosition(keyframes[KeyframeEndIndex - 1]->value);
                continue;
            }

            if (KeyframeEndIndex == 0) {
                xform->SetPosition(keyframes[0]->value);
                continue;
            }

            int KeyframeStartIndex = KeyframeEndIndex - 1;

            PositionKeyframe* startKeyframe = keyframes[KeyframeStartIndex];
            PositionKeyframe* endKeyframe = keyframes[KeyframeEndIndex];

            float percent = (m_ElapsedTime - startKeyframe->time) / (endKeyframe->time - startKeyframe->time);
            float factor = 0.f;
            switch (endKeyframe->easingType) {
                case EasingType::Linear:
                    factor = percent;
                    break;

                case EasingType::sineEaseIn:
                    factor = glm::sineEaseIn(percent);
                    break;

                case EasingType::sineEaseOut:
                    factor = glm::sineEaseOut(percent);
                    break;

                case EasingType::sineEaseInOut:
                    factor = glm::sineEaseInOut(percent);
                    break;
            }

            glm::vec3 interpolated = glm::mix(startKeyframe->value, endKeyframe->value, factor);
            xform->SetPosition(interpolated);
        }
    }

    /// SCALE
    for (auto& xformKeysPair : clip.xformScaleKeyframesMap) {
        Transform* xform = xformKeysPair.first;
        std::vector<ScaleKeyframe*>& keyframes = xformKeysPair.second;

        // If there is only 1 key frame, use that one.
        // If we are equal to or greater than the time of the last key frame, use the last keyframe
        if (keyframes.size() == 1) {
            xform->SetScale(keyframes[0]->value);
        } else {
            // Find our position keyframes (Start and End)
            int KeyframeEndIndex = 0;
            for (; KeyframeEndIndex < keyframes.size(); KeyframeEndIndex++) {
                if (keyframes[KeyframeEndIndex]->time > m_ElapsedTime) {
                    break;
                }
            }

            if (KeyframeEndIndex >= keyframes.size()) {
                // we are at or past the last key frame use the last keyframe only
                xform->SetScale(keyframes[KeyframeEndIndex - 1]->value);
                continue;
            }

            if (KeyframeEndIndex == 0) {
                xform->SetScale(keyframes[0]->value);
                continue;
            }

            int KeyFrameStartIndex = KeyframeEndIndex - 1;

            ScaleKeyframe* startKeyframe = keyframes[KeyFrameStartIndex];
            ScaleKeyframe* endKeyframe = keyframes[KeyframeEndIndex];

            float percent = (m_ElapsedTime - startKeyframe->time) / (endKeyframe->time - startKeyframe->time);
            float factor = 0.f;
            switch (endKeyframe->easingType) {
                case EasingType::Linear:
                    factor = percent;
                    break;

                case EasingType::sineEaseIn:
                    factor = glm::sineEaseIn(percent);
                    break;

                case EasingType::sineEaseOut:
                    factor = glm::sineEaseOut(percent);
                    break;

                case EasingType::sineEaseInOut:
                    factor = glm::sineEaseInOut(percent);
                    break;
            }

            glm::vec3 interpolated = glm::mix(startKeyframe->value, endKeyframe->value, factor);
            xform->SetScale(interpolated);
        }
    }

    /// ROTATION
    for (auto& xformKeysPair : clip.xformRotationKeyframesMap) {
        Transform* transform = xformKeysPair.first;
        std::vector<RotationKeyframe*>& keyframes = xformKeysPair.second;

        // If there is only 1 key frame, use that one.
        // If we are equal to or greater than the time of the last key frame, use the last keyframe
        if (keyframes.size() == 1) {
            transform->SetRotation(keyframes[0]->value);
        } else {
            // Find our rotation keyframes (Start and End)
            int KeyframeEndIndex = 0;
            for (; KeyframeEndIndex < keyframes.size(); KeyframeEndIndex++) {
                if (keyframes[KeyframeEndIndex]->time > m_ElapsedTime) {
                    break;
                }
            }

            if (KeyframeEndIndex >= keyframes.size()) {
                // we are at or past the last key frame use the last keyframe only
                transform->SetRotation(keyframes[KeyframeEndIndex - 1]->value);
                continue;
            }

            if (KeyframeEndIndex == 0) {
                transform->SetRotation(keyframes[0]->value);
                continue;
            }

            int KeyframeStartIndex = KeyframeEndIndex - 1;

            RotationKeyframe* startKeyframe = keyframes[KeyframeStartIndex];
            RotationKeyframe* endKeyFrame = keyframes[KeyframeEndIndex];

            float percent = (m_ElapsedTime - startKeyframe->time) / (endKeyFrame->time - startKeyframe->time);
            float factor = 0.f;
            switch (endKeyFrame->easingType) {
                case EasingType::Linear:
                    factor = percent;
                    break;

                case EasingType::sineEaseIn:
                    factor = glm::sineEaseIn(percent);
                    break;

                case EasingType::sineEaseOut:
                    factor = glm::sineEaseOut(percent);
                    break;

                case EasingType::sineEaseInOut:
                    factor = glm::sineEaseInOut(percent);
                    break;
            }

            glm::quat interpolated = glm::slerp(startKeyframe->value, endKeyFrame->value, factor);
            transform->SetRotation(interpolated);
        }
    }
}

void AnimationSystem::PlayPreviousClip() {
    SetCurrentClipIndex(0);
}

void AnimationSystem::PlayNextClip() {
    SetCurrentClipIndex(1);
}

}  // namespace gdp1