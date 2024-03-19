#pragma once

#include "common.h"
#include "bounds.h"

namespace gdp1 {

// forward declaration
class Collider;
class GameObject;
class SoftBody;

class Rigidbody {
public:
    glm::vec3 position;
    glm::quat orientation;

    glm::vec3 velocity;

    float invMass;  // inverse of mass, 0.0f means infinite mass (static object)

    Collider* collider;

    GameObject* object;
    bool active;

    glm::vec3 GetCenterOfMassWorldSpace() const;
    glm::vec3 GetCenterOfMassLocalSpace() const;

    glm::vec3 WorldSpaceToLocalSpace(const glm::vec3& pt) const;
    glm::vec3 LocalSpaceToWorldSpace(const glm::vec3& pt) const;

    void ApplyImpulse(const glm::vec3& impulse);

    Bounds GetBounds() const;
};

}  // namespace gdp1
