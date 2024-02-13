#pragma once

#include <glm/glm.hpp>

namespace gdp1 {

// forward declaration
class Rigidbody;
struct Contact;

bool Intersect(Rigidbody* a, Rigidbody* b, Contact& contact);
bool IntersectSphereSphere(Rigidbody* a, Rigidbody* b, Contact& contact);
bool IntersectSphereMesh(Rigidbody* a, Rigidbody* b, Contact& contact);

bool IntersectSphereTriangle(const glm::vec3& sphereCentre, float sphereRadius, const glm::vec3& vert0,
                             const glm::vec3& vert1, const glm::vec3& vert2, Contact& contact);

glm::vec3 IntersectRayTriangleRTCD(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

bool IntersectRayTriangleDX(const glm::vec3& orig, const glm::vec3& dir, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2,
                            float& t, float& u, float& v);

}  // namespace gdp1
