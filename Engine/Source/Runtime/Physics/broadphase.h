#pragma once

#include <vector>

#include "collision_info.h"

namespace gdp1 {

enum class SweepAxis { X, Y, Z };

void BroadPhase(std::vector<Rigidbody*>& bodies, std::vector<CollisionInfo>& collisions);

}  // namespace gdp1
