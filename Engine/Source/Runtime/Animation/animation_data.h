#pragma once

#include <string>
#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gdp1 {

// forward declaration
struct Transform;

enum class EasingType { Linear, sineEaseIn, sineEaseOut, sineEaseInOut };

struct PositionKeyframe {
    PositionKeyframe() = default;
    PositionKeyframe(const glm::vec3& position, float time, EasingType type = EasingType::Linear)
        : value(position)
        , time(time)
        , easingType(type) {
    }
    glm::vec3 value;
    EasingType easingType;
    float time;

    std::string objectName;
    // runtime data
    Transform* xform;
};

struct RotationKeyframe {
    RotationKeyframe() = default;
    RotationKeyframe(const glm::quat& rotation, float time, EasingType type = EasingType::Linear)
        : value(rotation)
        , time(time)
        , easingType(type) {
    }
    glm::quat value;
    EasingType easingType;
    float time;

    std::string objectName;
    // runtime data
    Transform* xform;
};

struct ScaleKeyframe {
    ScaleKeyframe() = default;
    ScaleKeyframe(const glm::vec3& scale, float time, EasingType type = EasingType::Linear)
        : value(scale)
        , time(time)
        , easingType(type) {
    }
    glm::vec3 value;
    EasingType easingType;
    float time;

    std::string objectName;
    // runtime data
    Transform* xform;
};

// struct AnimationChannel
// class AnimationSampler
// class AnimationEvaluator

// Stores keyframe based animations. (a.k.a. sequence)
struct AnimationClip {
    std::string name;
    float length;  // in seconds

    // std::vector<AnimationCurve> curves;
    std::vector<PositionKeyframe> positionKeys;
    std::vector<RotationKeyframe> rotationKeys;
    std::vector<ScaleKeyframe> scaleKeys;
};

class Animation {
public:
    std::string name;
    std::vector<AnimationClip> clips;
};

}  // namespace gdp1
