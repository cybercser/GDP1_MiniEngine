#pragma once

#include <vector>

#include "rigidbody.h"

namespace gdp1 {

enum class SweepAxis { X, Y, Z };

struct CollisionInfo {
    Rigidbody* body1;
    Rigidbody* body2;

    CollisionInfo(Rigidbody* body1, Rigidbody* body2)
        : body1(body1)
        , body2(body2) {
    }

    bool operator==(const CollisionInfo& other) const {
        return (body1 == other.body1 && body2 == other.body2) || (body1 == other.body2 && body2 == other.body1);
    }

    bool operator!=(const CollisionInfo& other) const {
        return !(*this == other);
    }
};

void BroadPhase(std::vector<Rigidbody*>& bodies, std::vector<CollisionInfo>& collisions);

}  // namespace gdp1
