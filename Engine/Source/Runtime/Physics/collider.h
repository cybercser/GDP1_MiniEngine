#pragma once

#include "common.h"
#include "bounds.h"

namespace gdp1 {

// Forward declaration
class GameObject;
class Transform;
class RigidBody;

class Collider {
public:
    enum class eShape : int {
        kUnknown,
        kSphere,
        kPlane,
        kBox,
        kCapsule,
        kMesh,
    };

    virtual eShape GetShapeType() const = 0;

    // All the GetBounds() methods return the bounds of the collider in world space.
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const = 0;
    virtual Bounds GetBounds(const glm::mat4& transMat) const = 0;
    virtual Bounds GetBounds() const = 0;

    glm::vec3 center;  // The center of the collider in object's local space.

    GameObject* gameObject;        // The game object this component is attached to.
    Transform* transform;          // The transform this collider is attached to.
    RigidBody* attachedRigidBody;  // The rigid body this collider is attached to.
    bool isTrigger;                // Specify if this collider is configured as a trigger.
};

//******************************************************************************
// SPHERE COLLIDER
//******************************************************************************
class SphereCollider : public Collider {
public:
    SphereCollider(const glm::vec3& c, float r)
        : radius(r) {
        center = c;
    }

    virtual eShape GetShapeType() const override { return eShape::kSphere; }
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    virtual Bounds GetBounds(const glm::mat4& transMat) const override;
    virtual Bounds GetBounds() const override;

    float radius;
};

//******************************************************************************
// BOX COLLIDER
//******************************************************************************
// A box collider works as an OBB (Oriented Bounding Box).
class BoxCollider : public Collider {
public:
    BoxCollider(const glm::vec3& c, const glm::vec3& s)
        : size(s) {
        center = c;
    }

    virtual eShape GetShapeType() const override { return eShape::kBox; }
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    virtual Bounds GetBounds(const glm::mat4& transMat) const override;
    virtual Bounds GetBounds() const override;

    glm::vec3 size;
};

//******************************************************************************
// CAPSULE COLLIDER
//******************************************************************************
class CapsuleCollider : public Collider {
public:
    CapsuleCollider(const glm::vec3& start, const glm::vec3& end, float radius)
        : m_Start(start)
        , m_End(end)
        , m_Radius(radius) {
        center = (start + end) * 0.5f;
    }

    virtual eShape GetShapeType() const override { return eShape::kCapsule; }
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    virtual Bounds GetBounds(const glm::mat4& transMat) const override;
    virtual Bounds GetBounds() const override;

    // The height of the capsule measured in the object's local space.
    // The capsule's height will be scaled by the transform's scale.
    // Note: The height is the actual height including the half-spheres at each end.
    float GetHeight() const { return glm::length(m_End - m_Start); }

    // The radius of the sphere, measured in the object's local space.
    // The capsule's radius will be scaled by the transform's scale.
    float GetRadius() const { return m_Radius; }

    const glm::vec3& GetStart() const { return m_Start; }
    const glm::vec3& GetEnd() const { return m_End; }

private:
    glm::vec3 m_Start, m_End;
    float m_Radius;
};

//******************************************************************************
// MESH COLLIDER
//******************************************************************************
struct ColliderVertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

struct ColliderTriangle {
    unsigned int indices[3];
};

struct ColliderMesh {
    std::vector<ColliderVertex> vertices;
    std::vector<ColliderTriangle> triangles;
};

class MeshCollider : public Collider {
public:
    MeshCollider(GameObject* pObject);

    virtual eShape GetShapeType() const override { return eShape::kMesh; }
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    virtual Bounds GetBounds(const glm::mat4& transMat) const override;
    virtual Bounds GetBounds() const override;

    const ColliderMesh& GetMesh() const { return m_Mesh; }

private:
    ColliderMesh m_Mesh;      // the mesh used for collision detection
    Bounds m_OriginalBounds;  // the non-transformed bounds of the mesh
};

}  // namespace gdp1
