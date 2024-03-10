#pragma once

#include <glm/glm.hpp>

namespace gdp1 {

// forward declaration
class Bounds;
struct PosTriangle;
class Collider;
class SphereCollider;
class BoxCollider;
class CapsuleCollider;
class MeshCollider;
struct Contact;

bool Intersect(Collider* a, Collider* b, Contact& contact);
bool IntersectSphereSphere(SphereCollider* a, SphereCollider* b, Contact& contact);
bool IntersectSphereBox(SphereCollider* sphere, BoxCollider* box, Contact& contact);
bool IntersectSphereCapsule(SphereCollider* sphere, CapsuleCollider* capsule, Contact& contact);
bool IntersectSphereMesh(SphereCollider* sphere, MeshCollider* mesh, Contact& contact);

bool IntersectBoundsSphere(const Bounds& bounds, SphereCollider* sphere);
bool IntersectBoundsBox(const glm::vec3& minA, BoxCollider* box);
bool IntersectBoundsCapsule(const Bounds& bounds, CapsuleCollider* capsule);
bool IntersectBoundsMesh(const Bounds& bounds, MeshCollider* mesh);
bool IntersectBoundsTriangle(const Bounds& bounds, const PosTriangle& tri);

bool IntersectSphereTriangle(const glm::vec3& sphereCentre, float sphereRadius, const glm::vec3& vert0,
                             const glm::vec3& vert1, const glm::vec3& vert2, Contact& contact);

glm::vec3 IntersectRayTriangleRTCD(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

bool IntersectRayTriangleDX(const glm::vec3& orig, const glm::vec3& dir, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2,
                            float& t, float& u, float& v);

}  // namespace gdp1
