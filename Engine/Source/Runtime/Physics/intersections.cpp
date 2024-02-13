
#include "intersections.h"

#include "rigidbody.h"
#include "contact.h"
#include "collider.h"
#include "Render/model.h"

namespace gdp1 {

bool Intersect(Rigidbody* a, Rigidbody* b, Contact& contact) {
    contact.bodyA = a;
    contact.bodyB = b;

    Collider::eShape shapeA = a->collider->GetShapeType();
    Collider::eShape shapeB = b->collider->GetShapeType();

    if (shapeA == Collider::eShape::SPHERE && shapeB == Collider::eShape::SPHERE) {
        return IntersectSphereSphere(a, b, contact);
    } else if (shapeA == Collider::eShape::SPHERE && shapeB == Collider::eShape::MESH) {
        return IntersectSphereMesh(a, b, contact);
    } else if (shapeB == Collider::eShape::SPHERE && shapeA == Collider::eShape::MESH) {
        return IntersectSphereMesh(b, a, contact);
    }

    return false;
}

bool IntersectSphereSphere(Rigidbody* a, Rigidbody* b, Contact& contact) {
    SphereCollider* sphereA = static_cast<SphereCollider*>(a->collider);
    SphereCollider* sphereB = static_cast<SphereCollider*>(b->collider);

    glm::vec3 ab = b->position - a->position;
    contact.normal = glm::normalize(ab);

    contact.ptOnA_WorldSpace = a->position + contact.normal * sphereA->radius;
    contact.ptOnB_WorldSpace = b->position - contact.normal * sphereB->radius;

    float radiusAB = sphereA->radius + sphereB->radius;
    float lengthSquare = glm::dot(ab, ab);
    if (lengthSquare <= radiusAB * radiusAB) {
        return true;
    }
    return false;
}

bool IntersectSphereMesh(Rigidbody* a, Rigidbody* b, Contact& contact) {
    SphereCollider* sphereCollider = static_cast<SphereCollider*>(a->collider);
    MeshCollider* meshCollider = static_cast<MeshCollider*>(b->collider);

    const ColliderMesh& hull = meshCollider->GetMesh();
    const std::vector<ColliderVertex>& vertices = hull.vertices;
    const std::vector<unsigned int> indices = hull.indices;
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int index0 = indices[i];
        unsigned int index1 = indices[i + 1];
        unsigned int index2 = indices[i + 2];

        const glm::vec3& p0 = vertices[index0].position;
        const glm::vec3& p1 = vertices[index1].position;
        const glm::vec3& p2 = vertices[index2].position;

        if (IntersectSphereTriangle(sphereCollider->centerOfMass, sphereCollider->radius, p0, p1, p2, contact)) {
            return true;
        }
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

// From: Real-Time Collision Detection- Ericson, Christer- 9781558607323- Books - Amazon.ca
// https://www.amazon.ca/Real-Time-Collision-Detection-Christer-Ericson/dp/1558607323/ref=pd_lpo_sccl_1/137-6663593-0701065?pd_rd_w=YiI8A&content-id=amzn1.sym.687e7c56-2b08-4044-894f-bbad969cf967&pf_rd_p=687e7c56-2b08-4044-894f-bbad969cf967&pf_rd_r=JWS56NJC99QEH56TYFJX&pd_rd_wg=zBE6V&pd_rd_r=d611733e-ec29-4b30-bd70-89f092f1991a&pd_rd_i=1558607323&psc=1
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
