#pragma once

#include "common.h"

#define RIGHT_VECTOR glm::vec4(1, 0, 0, 1);
#define UP_VECTOR glm::vec4(0, 1, 0, 1);
#define FORWARD_VECTOR glm::vec4(0, 0, -1, 1);

namespace gdp1 {
class TransformUtils {
public:

    static void ApplyTranslation(const glm::vec3& position, glm::mat4& matModelOut);

    static void ApplyRotation(const glm::quat& qOrientation, glm::mat4& matModelOut);

    static void ApplyScale(const float& scale, glm::mat4& matModelOut);

    static void GetTransform(const glm::vec3& position, const glm::quat& orientation, const float& scale,
                             glm::mat4& matModelOut);

    static void GetTransform(const glm::vec3& position, const float& scale, glm::mat4& matModelOut);

    static glm::vec3 GetUpVector(const glm::quat& orientation);

    static glm::vec3 GetForwardVector(const glm::quat& orientation);

    static glm::vec3 GetRightVector(const glm::quat& orientation);

    static glm::quat AdjustOrientation(const glm::quat& orientation, const glm::vec3& delta);

    static glm::vec3 GetQuatAsDegrees(const glm::quat& orientation);

    static glm::quat GetDegreesAsQuat(const glm::vec3& rotation);

    static glm::vec3 LocalToWorldPoint(const glm::vec3& point, const glm::vec3& position, const glm::quat& orientation,
                                       const float& scale, glm::mat4& parentMat = glm::mat4(1.0f));

    static glm::vec3 LocalToWorldPoint(const glm::vec3& point, const glm::vec3& position, const float& scale = 1.0f,
                                       glm::mat4& parentMat = glm::mat4(1.0f));

    static glm::vec3 WorldToLocalPoint(const glm::vec3& point, const glm::vec3& position, const glm::quat& orientation,
                                       const float& scale, glm::mat4& parentMat = glm::mat4(1.0f));

    static glm::vec3 WorldToLocalPoint(const glm::vec3& point, const glm::vec3& position, const float& scale,
                                       glm::mat4& parentMat = glm::mat4(1.0f));
};
}  // namespace gdp1
