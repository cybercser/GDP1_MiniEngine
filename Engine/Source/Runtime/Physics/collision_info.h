#pragma once

#include "rigidbody.h"
#include "softbody.h"

namespace gdp1 {

struct CollisionInfo {
    Rigidbody* body1;
    Rigidbody* body2;

    CollisionInfo(Rigidbody* body1, Rigidbody* body2)
        : body1(body1)
        , body2(body2) {}

    bool operator==(const CollisionInfo& other) const {
        return (body1 == other.body1 && body2 == other.body2) || (body1 == other.body2 && body2 == other.body1);
    }

    bool operator!=(const CollisionInfo& other) const { return !(*this == other); }
};

struct SoftBodyCollisionInfo {
    SoftBody* body1;
    Rigidbody* body2;

    SoftBodyCollisionInfo(SoftBody* body1, Rigidbody* body2)
        : body1(body1)
        , body2(body2) {}

    bool operator==(const SoftBodyCollisionInfo& other) const {
        return (body1 == other.body1 && body2 == other.body2);
    }

    bool operator!=(const SoftBodyCollisionInfo& other) const { return !(*this == other); }
};

}  // namespace gdp1