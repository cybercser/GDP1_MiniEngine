#pragma once

#include <glm/glm.hpp>
#include "Resource/level_object_descriptor.h"

namespace gdp1 {

// Forward declaration
class GameObject;

struct Transform {
    glm::vec3 localPosition;     // Position of the transform relative to the parent transform.
    glm::vec3 localEulerAngles;  // The rotation of the transform relative to the transform rotation of the parent.
    glm::vec3 localScale;        // The lossyScale of the transform relative to the GameObjects parent.

    glm::quat localRotation;  // The rotation of the transform relative to the transform rotation of the parent.

    Transform* parent;  // The parent of the transform.
    Transform* root;    // The topmost transform in the hierarchy.
    std::vector<Transform*> children;

    bool hasChanged;  // Has the transform changed since the last time the flag was set to 'false'?

    GameObject*
        gameObject;  // The game object this component is attached to. A component is always attached to a game object.

    Transform() = delete;
    Transform(GameObject* go);
    Transform(GameObject* go, const TransformDesc& desc);
    ~Transform();

    // getters and setters for position, eulerAngles, lossyScale and rotation
    glm::vec3 Position() const;
    void SetPosition(const glm::vec3& pos);
    glm::vec3 EulerAngles() const;  // The rotation as Euler angles in degrees.
    void SetEulerAngles(const glm::vec3& anglesInDegree);
    glm::vec3 Scale() const;
    void SetScale(const glm::vec3& scale);
    glm::quat Rotation() const;
    void SetRotation(const glm::quat& rot);

    glm::mat4 LocalMatrix() const;
    glm::mat4 WorldMatrix() const;
    void SetWorldMatrix(const glm::mat4& mat);

    glm::mat4 LocalToWorldMatrix()
        const;  // Matrix that transforms a point from local space into world space (Read Only).
    glm::mat4 WorldToLocalMatrix()
        const;  // Matrix that transforms a point from world space into local space (Read Only).

    void UpdateLocalMatrix();

private:
    glm::vec3 m_Position;  // The world space position of the Transform.
    glm::quat m_Rotation;  // A Quaternion that stores the rotation of the Transform in world space.
    glm::vec3 m_Scale;     // The global scale of the object (Read Only).

    glm::mat4 m_LocalMatrix;  // The local transform matrix.
    glm::mat4 m_WorldMatrix;  // The world transform matrix.

    bool m_NeedToUpdateLocalMatrix;    // Does the local matrix need to be updated?
    bool m_NeedToUpdateLocalRotation;  // Does the local rotation need to be updated?
};

}  // namespace gdp1
