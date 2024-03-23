#pragma once

#include "common.h"
#include "bounds.h"

namespace gdp1 {

// Forward declaration
class GameObject;

class Collider {
public:
    // How the physics item knows what type of object this is
    enum class eShape : int {
        UNKNOWN,
        SPHERE,
        PLANE,
        AABB,
        CAPSULE,
        MESH,
        POINT,
    };

    virtual eShape GetShapeType() const = 0;
    virtual Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const = 0;
    virtual Bounds GetBounds(const glm::mat4& transMat) const = 0;
    virtual Bounds GetBounds() const = 0;

    glm::vec3 centerOfMass;
};

class SphereCollider : public Collider {
public:
    SphereCollider(float r)
        : radius(r) {
        centerOfMass = glm::vec3(0.0);
    }

    eShape GetShapeType() const override {
        return eShape::SPHERE;
    }
    Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    Bounds GetBounds(const glm::mat4& transMat) const override;
    Bounds GetBounds() const override;

    float radius;
};

#if 0
class BoxCollider : public Collider {
public:
	BoxCollider(const glm::vec3& s)
		: size(s) {
		centerOfMass = glm::vec3(0.0);
	}

	eShape GetShapeType() const override {
		return eShape::AABB;
	}
	Bounds GetBounds(const glm::vec3& pos, const glm::vec3& lossyScale, const glm::quat& orient) const override;
	Bounds GetBounds(const glm::mat4& transMat) const override;
	Bounds GetBounds() const override;

	glm::vec3 size;
};
#endif

struct ColliderVertex {
    glm::vec3 position;
    glm::vec2 texCoords;
};

struct ColliderMesh {
    std::vector<ColliderVertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};

class MeshCollider : public Collider {
public:
    MeshCollider(GameObject* pObject);

    eShape GetShapeType() const override {
        return eShape::MESH;
    }
    Bounds GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const override;
    Bounds GetBounds(const glm::mat4& transMat) const override;
    Bounds GetBounds() const override;

    const ColliderMesh& GetMesh() const {
        return hull_;
    }

    GameObject* object;

private:
    void SetupMesh();

private:
    ColliderMesh hull_;
    Bounds bounds_;
};

// class CapsuleCollider

// class PlaneCollider

}  // namespace gdp1
