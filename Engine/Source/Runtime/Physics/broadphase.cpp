#include "broadphase.h"

#include <algorithm>
#include <functional>

#include "collider.h"

namespace gdp1 {

// find overlapping pairs by "sweep and prune"
void SweepAndPruneAxis(std::vector<Rigidbody*>& bodies, std::vector<CollisionInfo>& collisions, SweepAxis axis) {
    // find overlapping pairs
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = i + 1; j < bodies.size(); j++) {
            Bounds aBounds = bodies[i]->collider->GetBounds();
            Bounds bBounds = bodies[j]->collider->GetBounds();

            switch (axis) {
                case SweepAxis::X:
                    if (aBounds.GetMax().x > bBounds.GetMin().x) {
                        collisions.push_back(CollisionInfo(bodies[i], bodies[j]));
                    }
                    break;

                case SweepAxis::Y:
                    if (aBounds.GetMax().y > bBounds.GetMin().y) {
                        collisions.push_back(CollisionInfo(bodies[i], bodies[j]));
                    }
                    break;

                case SweepAxis::Z:
                    if (aBounds.GetMax().z > bBounds.GetMin().z) {
                        collisions.push_back(CollisionInfo(bodies[i], bodies[j]));
                    }
                    break;
            }
        }
    }
}

// find overlapping pairs by "sweep and prune"
void BroadPhase(std::vector<Rigidbody*>& bodies, std::vector<CollisionInfo>& collisions) {
    collisions.clear();
    // make a copy of the bodies_ vector
    std::vector<Rigidbody*> sortedBodies = bodies;

    // sort bodies_ by comparing bounds min x
    std::function<bool(Rigidbody*, Rigidbody*)> xComparator = [](Rigidbody* a, Rigidbody* b) {
        Bounds aBounds = a->collider->GetBounds();
        Bounds bBounds = b->collider->GetBounds();
        return aBounds.GetMin().x < bBounds.GetMin().x;
    };

    std::sort(sortedBodies.begin(), sortedBodies.end(), xComparator);
    SweepAndPruneAxis(sortedBodies, collisions, SweepAxis::X);

    // sort bodies_ by comparing bounds min y
    std::function<bool(Rigidbody*, Rigidbody*)> yComparator = [](Rigidbody* a, Rigidbody* b) {
        Bounds aBounds = a->collider->GetBounds();
        Bounds bBounds = b->collider->GetBounds();
        return aBounds.GetMin().y < bBounds.GetMin().y;
    };

    std::sort(sortedBodies.begin(), sortedBodies.end(), yComparator);
    SweepAndPruneAxis(sortedBodies, collisions, SweepAxis::Y);

    // sort bodies_ by comparing bounds min z
    std::function<bool(Rigidbody*, Rigidbody*)> zComparator = [](Rigidbody* a, Rigidbody* b) {
        Bounds aBounds = a->collider->GetBounds();
        Bounds bBounds = b->collider->GetBounds();
        return aBounds.GetMin().z < bBounds.GetMin().z;
    };

    std::sort(sortedBodies.begin(), sortedBodies.end(), zComparator);
    SweepAndPruneAxis(sortedBodies, collisions, SweepAxis::Z);
}

}  // namespace gdp1
