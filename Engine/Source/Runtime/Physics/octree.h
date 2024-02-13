#pragma once

#include <vector>
#include <array>
#include <memory>

#include "bounds.h"

namespace gdp1 {

// forward declaration
class Collider;
class Shader;

// inspired by this tutorial [Dividing 3D Space into an Octree](https://www.youtube.com/watch?v=JvWdyPf0aGw)
// remember, the octree node is always a cube
struct OctreeNode {
    Bounds bounds;
    std::array<Bounds, 8> child_bounds;
    unsigned int level;
    std::vector<OctreeNode*> children;

    static unsigned int MAX_LEVEL;
    static float MIN_SIZE;

    OctreeNode(Bounds bounds, unsigned int level);
    ~OctreeNode();

    void AddCollider(Collider* collider);

    void Draw(std::shared_ptr<Shader> shader);

private:
    void DivideAndAdd(Collider* collider);

    void SetupDebugData();

    // debug drawing stuff
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};

class Octree {
public:
    Octree(std::vector<Collider*> colliders, unsigned max_level, float min_size);

    ~Octree();

    void AddColliders(std::vector<Collider*> colliders);

    void Draw(std::shared_ptr<Shader> shader);

private:
    OctreeNode* root;
};

}  // namespace gdp1
