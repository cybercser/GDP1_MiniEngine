#pragma once

#include <vector>

#include "collision_info.h"

namespace gdp1 {

enum class SweepAxis { X, Y, Z };

void BroadPhase(std::vector<Rigidbody*>& bodies, std::vector<CollisionInfo>& collisions);

void BroadPhaseSoftBodies(std::vector<SoftBody*>& softbodies, std::vector<Rigidbody*>& rigidbodies,
                          std::vector<SoftBodyCollisionInfo>& collisions);

void SweepAndPruneAxisSoftBody(std::vector<SoftBody*>& softbodies, std::vector<Rigidbody*>& rigidbodies,
                               std::vector<SoftBodyCollisionInfo>& collisions, SweepAxis axis);

}  // namespace gdp1
