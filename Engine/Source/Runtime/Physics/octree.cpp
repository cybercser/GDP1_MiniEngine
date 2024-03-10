#include "octree.h"

#include <glm/glm.hpp>

#include "collider.h"
#include "Core/game_object.h"
#include "Render/model.h"
#include "Physics/intersections.h"

using namespace glm;

namespace gdp1 {
// when the number of triangles in a node is less than this number, we stop splitting the node
unsigned int OctreeNode::MIN_TRIANGLES_IN_NODE = 8;

OctreeNode::OctreeNode(Bounds bounds, unsigned int level) {
    this->bounds = bounds;
    this->level = level;

    // split the node into 8 children
    vec3 half_size = bounds.GetSize() * 0.5f;

    // learned from three.js octree
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {
                Bounds box = bounds;
                vec3 v = vec3(x, y, z);
                vec3 newMin = bounds.GetMin() + v * half_size;
                vec3 newMax = newMin + half_size;
                child_bounds.emplace_back(Bounds(newMin, newMax));
            }
        }
    }
}

OctreeNode::~OctreeNode() {
    for (auto child : children) {
        delete child;
    }
}

void OctreeNode::SplitAndAdd(Collider* collider) {
    if (collider == nullptr) return;

    Bounds colliderBounds = collider->GetBounds();
    for (int i = 0; i < 8; i++) {
        bool intersects = false;

        if (child_bounds[i].Intersects(colliderBounds)) {
            Collider::eShape shape = collider->GetShape();
            if (shape == Collider::eShape::kSphere) {
                SphereCollider* sphereCollider = dynamic_cast<SphereCollider*>(collider);
                if (sphereCollider != nullptr) {
                    intersects = IntersectBoundsSphere(child_bounds[i], sphereCollider);
                }
            } else if (shape == Collider::eShape::kBox) {
                BoxCollider* boxCollider = dynamic_cast<BoxCollider*>(collider);
                if (boxCollider != nullptr) {
                    intersects = IntersectBoundsBox(child_bounds[i].GetMin(), boxCollider);
                }
            } else if (shape == Collider::eShape::kCapsule) {
                CapsuleCollider* capsuleCollider = dynamic_cast<CapsuleCollider*>(collider);
                if (capsuleCollider != nullptr) {
                    intersects = IntersectBoundsCapsule(child_bounds[i], capsuleCollider);
                }
            } else if (shape == Collider::eShape::kMesh) {
                MeshCollider* meshCollider = dynamic_cast<MeshCollider*>(collider);
                if (meshCollider != nullptr) {
                    intersects = IntersectBoundsMesh(child_bounds[i], meshCollider);
                }
            }
        }

        if (intersects) {
            OctreeNode* child = new OctreeNode(child_bounds[i], level + 1);
            children.push_back(child);

            // move the triangles that intersect with the child bounds to the child
            while (!triangles.empty()) {
                PosTriangle tri = triangles.back();
                triangles.pop_back();

                if (IntersectBoundsTriangle(child_bounds[i], tri)) {
                    child->triangles.push_back(tri);
                }
            }

            // recursively split the children if:
            // 1. the number of triangles in the node is greater than MIN_TRIANGLES_IN_NODE
            // 2. and the level is less than MAX_LEVEL
            if (children[i]->triangles.size() > MIN_TRIANGLES_IN_NODE && level < MAX_LEVEL)
                children[i]->SplitAndAdd(collider);
        }
    }
}

Octree::Octree(const std::vector<Collider*>& colliders, uint32_t minTrianglesInNode, uint32_t maxLevel) {
    // Calculate a bounding box that contains all the colliders
    Bounds bounds;
    for (auto collider : colliders) {
        Bounds b = collider->GetBounds();
        bounds.Expand(b);
    }
    // offset the bounds a little bit to account for very thin slate-like objects
    bounds.Expand(vec3(0.01f));

    m_Root = new OctreeNode(bounds, 0);
    m_Root->MIN_TRIANGLES_IN_NODE = minTrianglesInNode;
    m_Root->MAX_LEVEL = maxLevel;

    // gather all the triangles from the colliders
    std::vector<unsigned int> indices(3);
    for (auto collider : colliders) {
        GameObject* go = collider->gameObject;
        Model* model = go->model;
        if (model != nullptr) {
            for (auto& mesh : model->meshes) {
                for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
                    PosTriangle tri;
                    indices[0] = mesh.indices[i];
                    indices[1] = mesh.indices[i + 1];
                    indices[2] = mesh.indices[i + 2];
                    tri.v0 = mesh.vertices[indices[0]].position;
                    tri.v1 = mesh.vertices[indices[1]].position;
                    tri.v2 = mesh.vertices[indices[2]].position;
                    // transform the triangle to world space
                    // #TODO: take moving objects into account
                    mat4 worldMat = go->transform->WorldMatrix();
                    tri.v0 = worldMat * vec4(tri.v0, 1.0f);
                    tri.v1 = worldMat * vec4(tri.v1, 1.0f);
                    tri.v2 = worldMat * vec4(tri.v2, 1.0f);

                    m_Root->triangles.push_back(tri);
                }
            }
        }
    }

    AddColliders(colliders);
}

Octree::~Octree() { delete m_Root; }

void Octree::AddColliders(const std::vector<Collider*>& colliders) {
    for (auto collider : colliders) {
        m_Root->SplitAndAdd(collider);
    }
}

}  // namespace gdp1
