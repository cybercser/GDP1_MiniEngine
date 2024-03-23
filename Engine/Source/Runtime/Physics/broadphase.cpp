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

void BroadPhaseSoftBodies(std::vector<SoftBody*>& softbodies, std::vector<Rigidbody*>& rigidbodies,
                std::vector<SoftBodyCollisionInfo>& collisions) {
    collisions.clear();

    // Make a copy of the soft bodies and rigid bodies vectors
    std::vector<SoftBody*> sortedSoftBodies = softbodies;
    std::vector<Rigidbody*> sortedRigidBodies = rigidbodies;

    // Sort soft bodies and rigid bodies separately along each axis
    // X-axis sorting for soft bodies
    std::function<bool(SoftBody*, SoftBody*)> softBodyXComparator = [](SoftBody* a, SoftBody* b) {
        Bounds aBounds = a->collider->GetBounds();  // Assuming SoftBody has a GetBounds method
        Bounds bBounds = b->collider->GetBounds();  // Adjust this based on your SoftBody implementation
        return aBounds.GetMin().x < bBounds.GetMin().x;
    };

    std::sort(sortedSoftBodies.begin(), sortedSoftBodies.end(), softBodyXComparator);
    SweepAndPruneAxisSoftBody(sortedSoftBodies, sortedRigidBodies, collisions, SweepAxis::X);

    // Y-axis sorting for soft bodies
    std::function<bool(SoftBody*, SoftBody*)> softBodyYComparator = [](SoftBody* a, SoftBody* b) {
        Bounds aBounds = a->collider->GetBounds();
        Bounds bBounds = b->collider->GetBounds();
        return aBounds.GetMin().y < bBounds.GetMin().y;
    };

    std::sort(sortedSoftBodies.begin(), sortedSoftBodies.end(), softBodyYComparator);
    SweepAndPruneAxisSoftBody(sortedSoftBodies, sortedRigidBodies, collisions, SweepAxis::Y);

    // Z-axis sorting for soft bodies
    std::function<bool(SoftBody*, SoftBody*)> softBodyZComparator = [](SoftBody* a, SoftBody* b) {
        Bounds aBounds = a->collider->GetBounds();
        Bounds bBounds = b->collider->GetBounds();
        return aBounds.GetMin().z < bBounds.GetMin().z;
    };

    std::sort(sortedSoftBodies.begin(), sortedSoftBodies.end(), softBodyZComparator);
    SweepAndPruneAxisSoftBody(sortedSoftBodies, sortedRigidBodies, collisions, SweepAxis::Z);
}

void SweepAndPruneAxisSoftBody(std::vector<SoftBody*>& softbodies, std::vector<Rigidbody*>& rigidbodies,
                               std::vector<SoftBodyCollisionInfo>& collisions, SweepAxis axis) {

    /*for (SoftBody* softBody : softbodies) {
        for (Rigidbody* rigidBody : rigidbodies) {
            if (Overlaps(softBody, rigidBody)) {
                SoftBodyCollisionInfo collision{};
                collision.bodyA = softBody;
                collision.bodyB = rigidBody;
                collisions.push_back(collision);
            }
        }
    }*/

    for (int i = 0; i < softbodies.size(); i++) {
        for (int j = i + 1; j < rigidbodies.size(); j++) {
            Bounds aBounds = softbodies[i]->collider->GetBounds();
            Bounds bBounds = rigidbodies[j]->collider->GetBounds();

            switch (axis) {
                case SweepAxis::X:
                    if (aBounds.GetMax().x > bBounds.GetMin().x) {
                        collisions.push_back(SoftBodyCollisionInfo(softbodies[i], rigidbodies[j]));
                    }
                    break;

                case SweepAxis::Y:
                    if (aBounds.GetMax().y > bBounds.GetMin().y) {
                        collisions.push_back(SoftBodyCollisionInfo(softbodies[i], rigidbodies[j]));
                    }
                    break;

                case SweepAxis::Z:
                    if (aBounds.GetMax().z > bBounds.GetMin().z) {
                        collisions.push_back(SoftBodyCollisionInfo(softbodies[i], rigidbodies[j]));
                    }
                    break;
            }
        }
    }
}

}  // namespace gdp1
