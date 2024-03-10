
#include "intersections.h"

#include <glm/gtc/matrix_transform.hpp>

#include "contact.h"
#include "collider.h"
#include "Render/model.h"
#include "Core/transform.h"
#include "bounds.h"
#include "Math/triangle.h"

namespace gdp1 {

bool Intersect(Collider* a, Collider* b, Contact& contact) {
    contact.colliderA = a;
    contact.colliderB = b;

    Collider::eShape shapeA = a->GetShape();
    Collider::eShape shapeB = b->GetShape();

    // sphere - sphere
    if (shapeA == Collider::eShape::kSphere && shapeB == Collider::eShape::kSphere) {
        SphereCollider* sa = dynamic_cast<SphereCollider*>(a);
        SphereCollider* sb = dynamic_cast<SphereCollider*>(b);
        if (sa == nullptr || sb == nullptr) return false;

        return IntersectSphereSphere(sa, sb, contact);
    }
    // sphere - box
    else if (shapeA == Collider::eShape::kSphere && shapeB == Collider::eShape::kBox) {
        SphereCollider* sphere = dynamic_cast<SphereCollider*>(a);
        BoxCollider* box = dynamic_cast<BoxCollider*>(b);
        if (sphere == nullptr || box == nullptr) return false;
        return IntersectSphereBox(sphere, box, contact);
    }
    // box - sphere
    else if (shapeA == Collider::eShape::kBox && shapeB == Collider::eShape::kSphere) {
        SphereCollider* sphere = dynamic_cast<SphereCollider*>(b);
        BoxCollider* box = dynamic_cast<BoxCollider*>(a);
        if (sphere == nullptr || box == nullptr) return false;
        return IntersectSphereBox(sphere, box, contact);
    }
    // sphere - mesh
    else if (shapeA == Collider::eShape::kSphere && shapeB == Collider::eShape::kMesh) {
        SphereCollider* sphere = dynamic_cast<SphereCollider*>(a);
        MeshCollider* mesh = dynamic_cast<MeshCollider*>(b);
        if (sphere == nullptr || mesh == nullptr) return false;
        return IntersectSphereMesh(sphere, mesh, contact);
    }
    // mesh - sphere
    else if (shapeB == Collider::eShape::kSphere && shapeA == Collider::eShape::kMesh) {
        SphereCollider* sphere = dynamic_cast<SphereCollider*>(b);
        MeshCollider* mesh = dynamic_cast<MeshCollider*>(a);
        if (sphere == nullptr || mesh == nullptr) return false;
        return IntersectSphereMesh(sphere, mesh, contact);
    }

    return false;
}

bool IntersectSphereSphere(SphereCollider* a, SphereCollider* b, Contact& contact) {
    glm::vec3 ab = b->center - a->center;
    contact.normal = glm::normalize(ab);

    contact.ptOnA_WorldSpace = a->center + contact.normal * a->radius;
    contact.ptOnB_WorldSpace = b->center - contact.normal * b->radius;

    float radiusAB = a->radius + b->radius;
    float lengthSquare = glm::dot(ab, ab);
    if (lengthSquare <= radiusAB * radiusAB) {
        return true;
    }
    return false;
}

bool IntersectSphereBox(SphereCollider* sphere, BoxCollider* box, Contact& contact) {
    // Transform the sphere center to the box's local space
    glm::vec3 sphereCenterLocal = glm::inverse(box->transform->WorldMatrix()) * glm::vec4(sphere->center, 1.0f);

    // Find the point on the box that is closest to the sphere
    glm::vec3 extents = box->GetBounds().GetExtents();
    glm::vec3 closestPoint = glm::clamp(sphereCenterLocal, -extents, extents);

    // Transform the closest point back to the world space
    closestPoint = box->transform->WorldMatrix() * glm::vec4(closestPoint, 1.0f);

    // Sphere and box intersect if the (squared) distance from sphere center to the closest point
    // is less than the (squared) sphere radius
    glm::vec3 v = closestPoint - sphere->center;
    contact.separationDist = glm::length(v);
    contact.ptOnA_WorldSpace = contact.ptOnB_WorldSpace = closestPoint;
    contact.normal = glm::normalize(v);

    if (glm::dot(v, v) <= (sphere->radius * sphere->radius)) {
        return true;
    }

    return false;
}

bool IntersectSphereCapsule(SphereCollider* sphere, CapsuleCollider* capsule, Contact& contact) { return false; }

bool IntersectSphereMesh(SphereCollider* sphere, MeshCollider* mesh, Contact& contact) {
    const ColliderMesh& hull = mesh->GetMesh();
    const std::vector<PNTVertex>& vertices = hull.vertices;
    for (auto& tri : hull.triangles) {
        const glm::vec3& p0 = vertices[tri.indices[0]].position;
        const glm::vec3& p1 = vertices[tri.indices[1]].position;
        const glm::vec3& p2 = vertices[tri.indices[2]].position;

        if (IntersectSphereTriangle(sphere->center, sphere->radius, p0, p1, p2, contact)) {
            return true;
        }
    }

    return false;
}

bool IntersectBoundsSphere(const Bounds& bounds, SphereCollider* sphere) { return false; }

bool IntersectBoundsBox(const glm::vec3& minA, BoxCollider* box) { return false; }

bool IntersectBoundsCapsule(const Bounds& bounds, CapsuleCollider* capsule) { return false; }

bool IntersectBoundsMesh(const Bounds& bounds, MeshCollider* mesh) {
    const ColliderMesh& hull = mesh->GetMesh();
    for (auto& vertex : hull.vertices) {
        if (bounds.Contains(vertex.position)) {
            return true;
        }
    }
    return false;
}

bool IntersectBoundsTriangle(const Bounds& bounds, const PosTriangle& tri) {
    if (bounds.Contains(tri.v0) || bounds.Contains(tri.v1) || bounds.Contains(tri.v2)) {
        return true;
    }

    return false;
}

//== Section 5.2.7: ============================================================
// Returns true if sphere s intersects triangle ABC, false otherwise.
bool IntersectSphereTriangle(const glm::vec3& sphereCentre, float sphereRadius, const glm::vec3& vert0,
                             const glm::vec3& vert1, const glm::vec3& vert2, Contact& contact) {
    // Find point P on triangle ABC closest to sphere center
    glm::vec3 closestPoint = IntersectRayTriangleRTCD(sphereCentre, vert0, vert1, vert2);

    // Sphere and triangle intersect if the (squared) distance from sphere
    // center to point p is less than the (squared) sphere radius
    glm::vec3 v = closestPoint - sphereCentre;
    contact.separationDist = glm::length(v);
    contact.ptOnA_WorldSpace = contact.ptOnB_WorldSpace = closestPoint;
    contact.normal = glm::normalize(v);

    bool isItIntersecting = false;

    if (glm::dot(v, v) <= (sphereRadius * sphereRadius)) {
        isItIntersecting = true;
    }

    return isItIntersecting;
}

// From: Real-Time Collision Detection- Ericson, Christer- 9781558607323
// https://www.amazon.ca/Real-Time-Collision-Detection-Christer-Ericson/dp/1558607323
// Chapter 5:
glm::vec3 IntersectRayTriangleRTCD(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 bc = c - b;

    // Compute parametric position s for projection P' of P on AB,
    // P' = A + s*AB, s = snom/(snom+sdenom)
    float snom = glm::dot(p - a, ab), sdenom = glm::dot(p - b, a - b);

    // Compute parametric position t for projection P' of P on AC,
    // P' = A + t*AC, s = tnom/(tnom+tdenom)
    float tnom = glm::dot(p - a, ac), tdenom = glm::dot(p - c, a - c);

    if (snom <= 0.0f && tnom <= 0.0f) return a;  // Vertex region early out

    // Compute parametric position u for projection P' of P on BC,
    // P' = B + u*BC, u = unom/(unom+udenom)
    float unom = glm::dot(p - b, bc), udenom = glm::dot(p - c, b - c);

    if (sdenom <= 0.0f && unom <= 0.0f) return b;    // Vertex region early out
    if (tdenom <= 0.0f && udenom <= 0.0f) return c;  // Vertex region early out

    // P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
    glm::vec3 n = glm::cross(b - a, c - a);
    float vc = glm::dot(n, glm::cross(a - p, b - p));
    // If P outside AB and within feature region of AB,
    // return projection of P onto AB
    if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f) return a + snom / (snom + sdenom) * ab;

    // P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
    float va = glm::dot(n, glm::cross(b - p, c - p));
    // If P outside BC and within feature region of BC,
    // return projection of P onto BC
    if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f) return b + unom / (unom + udenom) * bc;

    // P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
    float vb = glm::dot(n, glm::cross(c - p, a - p));
    // If P outside CA and within feature region of CA,
    // return projection of P onto CA
    if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f) return a + tnom / (tnom + tdenom) * ac;

    // P must project inside face region. Compute Q using barycentric coordinates
    float u = va / (va + vb + vc);
    float v = vb / (va + vb + vc);
    float w = 1.0f - u - v;  // = vc / (va + vb + vc)
    return u * a + v * b + w * c;
}

// DirectX sdk legacy samples - Pick
// https://github.com/walbourn/directx-sdk-legacy-samples/blob/main/Direct3D10/Pick10/Pick10.cpp#L869
bool IntersectRayTriangleDX(const glm::vec3& orig, const glm::vec3& dir, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2,
                            float& t, float& u, float& v) {
    // Find vectors for two edges sharing vert0
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate U parameter
    glm::vec3 pvec = glm::cross(dir, edge2);

    // If determinant is near zero, ray lies in plane of triangle
    float det = glm::dot(edge1, pvec);

    glm::vec3 tvec;
    if (det > 0) {
        tvec = orig - v0;
    } else {
        tvec = v0 - orig;
        det = -det;
    }

    if (det < 0.0001f) return false;

    // Calculate U parameter and test bounds
    u = glm::dot(tvec, pvec);
    if (u < 0.0f || u > det) return false;

    // Prepare to test V parameter
    glm::vec3 qvec = glm::cross(tvec, edge1);

    // Calculate V parameter and test bounds
    v = glm::dot(dir, qvec);
    if (v < 0.0f || u + v > det) return false;

    // Calculate t, lossyScale parameters, ray intersects triangle
    t = glm::dot(edge2, qvec);
    float fInvDet = 1.0f / det;
    t *= fInvDet;
    u *= fInvDet;
    v *= fInvDet;

    return true;
}

}  // namespace gdp1
