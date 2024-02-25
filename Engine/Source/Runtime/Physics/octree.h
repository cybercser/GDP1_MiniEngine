#pragma once

#include <vector>
#include <list>

#include "bounds.h"
#include "Math/triangle.h"

namespace gdp1 {

// forward declaration
class Collider;
class Shader;

// inspired by this tutorial [Dividing 3D Space into an Octree](https://www.youtube.com/watch?v=JvWdyPf0aGw)
// but the bounds are not necessarily cubes, just regular AABBs.
// Also inspired by [three.js octree](https://github.com/mrdoob/three.js/blob/dev/examples/jsm/math/Octree.js)
struct OctreeNode {
    Bounds bounds;
    std::vector<Bounds> child_bounds;
    uint32_t level;
    std::vector<OctreeNode*> children;

    std::list<PosTriangle> triangles;

    static uint32_t MIN_TRIANGLES_IN_NODE;
    static uint32_t MAX_LEVEL;

    OctreeNode(Bounds bounds, uint32_t level);
    ~OctreeNode();

    void SplitAndAdd(Collider* collider);
};

class Octree {
public:
    Octree(const std::vector<Collider*>& colliders, uint32_t minTrianglesInNode, uint32_t maxLevel);

    ~Octree();

    void AddColliders(const std::vector<Collider*>& colliders);

private:
    OctreeNode* m_Root;
    uint32_t m_MaxLevel;
};

}  // namespace gdp1
