#pragma once

#include "common.h"

namespace gdp1 {

// Forward declaration
class Collider;

struct Contact {
    glm::vec3 ptOnA_WorldSpace;
    glm::vec3 ptOnB_WorldSpace;
    glm::vec3 ptOnA_LocalSpace;
    glm::vec3 ptOnB_LocalSpace;

    glm::vec3 normal;      // in world space
    float separationDist;  // positive when non-penetrating, negative when penetrating
    float timeOfImpact;

    Collider* colliderA;
    Collider* colliderB;
};

// void ResolveContact(Contact& contact);

}  // namespace gdp1
