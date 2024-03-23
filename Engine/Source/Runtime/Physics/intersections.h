#pragma once

#include <glm/glm.hpp>

namespace gdp1 {

// forward declaration
class Rigidbody;
class SoftBody;
struct Contact;
struct SoftBodyContact;

bool Intersect(SoftBody* a, Rigidbody* b, SoftBodyContact& contact);
bool Intersect(Rigidbody* a, Rigidbody* b, Contact& contact);
bool IntersectSphereSphere(Rigidbody* a, Rigidbody* b, Contact& contact);
bool IntersectSphereMesh(Rigidbody* a, Rigidbody* b, Contact& contact);

bool IntersectPointMesh(SoftBody* a, Rigidbody* b, SoftBodyContact& contact);
bool IntersectPointSphere(SoftBody* a, Rigidbody* b, SoftBodyContact& contact);

bool IntersectSphereTriangle(const glm::vec3& sphereCentre, float sphereRadius, const glm::vec3& vert0,
                             const glm::vec3& vert1, const glm::vec3& vert2, Contact& contact);

glm::vec3 IntersectRayTriangleRTCD(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

bool IntersectRayTriangleDX(const glm::vec3& orig, const glm::vec3& dir, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2,
                            float& t, float& u, float& v);

bool IntersectPointTriangle(const glm::vec3& point, const glm::vec3& vert0, const glm::vec3& vert1,
                            const glm::vec3& vert2, SoftBodyContact& contact);

}  // namespace gdp1
