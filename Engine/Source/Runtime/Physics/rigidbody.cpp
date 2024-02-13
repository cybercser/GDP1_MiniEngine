#include "collider.h"

#include <iostream>

#include "rigidbody.h"

namespace gdp1 {

glm::vec3 Rigidbody::GetCenterOfMassWorldSpace() const {
    glm::vec3 centerOfMass = collider->centerOfMass;
    glm::vec3 pos = position + orientation * centerOfMass;
    return pos;
}

glm::vec3 Rigidbody::GetCenterOfMassLocalSpace() const {
    return collider->centerOfMass;
}

glm::vec3 Rigidbody::WorldSpaceToLocalSpace(const glm::vec3& worldPt) const {
    glm::vec3 tmp = worldPt - GetCenterOfMassWorldSpace();
    glm::quat inverseOrient = glm::inverse(orientation);
    glm::vec3 localSpace = inverseOrient * tmp;
    return localSpace;
}

glm::vec3 Rigidbody::LocalSpaceToWorldSpace(const glm::vec3& bodyPt) const {
    glm::vec3 worldSpace = GetCenterOfMassWorldSpace() + orientation * bodyPt;
    return worldSpace;
}

void Rigidbody::ApplyImpulse(const glm::vec3& impulse) {
    if (0.0f == invMass || !active) return;

    velocity += impulse * invMass;
}

Bounds Rigidbody::GetBounds() const {
    assert(collider != nullptr);
    return collider->GetBounds();
}

}  // namespace gdp1
