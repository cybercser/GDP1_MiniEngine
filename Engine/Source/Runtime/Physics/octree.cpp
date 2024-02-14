#include "octree.h"

#include "collider.h"
#include "Render/shader.h"

namespace gdp1 {
unsigned int OctreeNode::MAX_LEVEL = 5;
float OctreeNode::MIN_SIZE = 0.1f;

OctreeNode::OctreeNode(Bounds bounds, unsigned int level) {
    this->bounds = bounds;
    this->level = level;

    SetupDebugData();

    // split the node into 8 children
    glm::vec3 child_size = bounds.GetSize() / 2.0f;
    float e = child_size.x / 2.0f;  // child extent

    glm::vec3 c = bounds.GetCenter();
    /*
    * the upper 4 children
    *   +---+---+
        | 0 | 1 |
        +---+---+
        | 2 | 3 |
        +---+---+
    */
    child_bounds[0] = Bounds(c + glm::vec3(-e, e, -e), child_size);
    child_bounds[1] = Bounds(c + glm::vec3(e, e, -e), child_size);
    child_bounds[2] = Bounds(c + glm::vec3(-e, e, e), child_size);
    child_bounds[3] = Bounds(c + glm::vec3(e, e, e), child_size);
    /*
    * the lower 4 children
    *   +---+---+
        | 5 | 6 |
        +---+---+
        | 7 | 8 |
        +---+---+
    */
    child_bounds[4] = Bounds(c + glm::vec3(-e, -e, -e), child_size);
    child_bounds[5] = Bounds(c + glm::vec3(e, -e, -e), child_size);
    child_bounds[6] = Bounds(c + glm::vec3(-e, -e, e), child_size);
    child_bounds[7] = Bounds(c + glm::vec3(e, -e, e), child_size);
}

OctreeNode::~OctreeNode() {
    for (auto child : children) {
        delete child;
    }
}

void OctreeNode::AddCollider(Collider* collider) { DivideAndAdd(collider); }

void OctreeNode::Draw(std::shared_ptr<Shader> shader) {
    shader->Use();
    shader->SetUniform("u_Model", glm::mat4(1.0f));
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for (auto child : children) {
        if (child != nullptr) child->Draw(shader);
    }
}

void OctreeNode::DivideAndAdd(Collider* collider) {
    if (level >= MAX_LEVEL || bounds.GetSize().x <= MIN_SIZE) {
        return;
    }

    if (children.empty()) {
        children.resize(8, nullptr);
    }

    bool dividing = false;
    for (int i = 0; i < 8; i++) {
        if (child_bounds[i].Intersects(
                collider->GetBounds())) {  // first check if the collider intersects with the child bounds
            // then check if the mesh intersects with the child bounds
            // SPHERE COLLIDER
            // AABB COLLIDER
            // MESH COLLIDER
            // #TODO: here we only deal with mesh colliders
            MeshCollider* meshCollider = dynamic_cast<MeshCollider*>(collider);
            bool intersects = false;
            if (meshCollider != nullptr) {
                const ColliderMesh& hull = meshCollider->GetMesh();
                for (auto& vertex : hull.vertices) {
                    if (child_bounds[i].Contains(vertex.position)) {
                        intersects = true;
                        break;
                    }
                }
            }

            if (!intersects) continue;

            if (children[i] == nullptr) {
                children[i] = new OctreeNode(child_bounds[i], level + 1);
            }

            dividing = true;
            children[i]->DivideAndAdd(collider);
        }
    }

    if (!dividing) {
        children.clear();
    }
}

#define ADD_LINE(a, b)    \
    indices.push_back(a); \
    indices.push_back(b);

void OctreeNode::SetupDebugData() {
    // calculate the 8 vertices of the bounding box
    // Y-up Z-forward X-right
    /*
   .2------6
 .'      .'|
3------7'  |
|      |   |
|      |   4
|      | .'
 1------5'

   .2------6
 .' |      |
3   |      |
|   |      |
|  .0------4
|.'      .'
 1------5'
    */

    glm::vec3 c = bounds.GetCenter();
    glm::vec3 e = bounds.GetExtents();
    vertices.push_back(c + glm::vec3(-e.x, -e.y, -e.z));  // 0
    vertices.push_back(c + glm::vec3(-e.x, -e.y, e.z));   // 1
    vertices.push_back(c + glm::vec3(-e.x, e.y, -e.z));   // 2
    vertices.push_back(c + glm::vec3(-e.x, e.y, e.z));    // 3
    vertices.push_back(c + glm::vec3(e.x, -e.y, -e.z));   // 4
    vertices.push_back(c + glm::vec3(e.x, -e.y, e.z));    // 5
    vertices.push_back(c + glm::vec3(e.x, e.y, -e.z));    // 6
    vertices.push_back(c + glm::vec3(e.x, e.y, e.z));     // 7

    // calculate the indices of the bounding box
    ADD_LINE(0, 1);
    ADD_LINE(0, 2);
    ADD_LINE(0, 4);
    ADD_LINE(1, 3);
    ADD_LINE(1, 5);
    ADD_LINE(2, 3);
    ADD_LINE(2, 6);
    ADD_LINE(3, 7);
    ADD_LINE(4, 5);
    ADD_LINE(4, 6);
    ADD_LINE(5, 7);
    ADD_LINE(6, 7);

    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

Octree::Octree(std::vector<Collider*> colliders, unsigned max_level, float min_size) {
    // Calculate a bounding box that contains all the colliders
    Bounds bounds;
    for (auto collider : colliders) {
        Bounds b = collider->GetBounds();
        bounds.Expand(b);
    }
    // make the bounds a cube
    glm::vec3 size = bounds.GetSize();
    float max_size = std::max(size.x, std::max(size.y, size.z));
    glm::vec3 sizeVec(max_size);
    bounds.SetMinMax(bounds.GetCenter() - sizeVec / 2.0f, bounds.GetCenter() + sizeVec / 2.0f);

    root = new OctreeNode(bounds, 1);
    root->MAX_LEVEL = max_level;
    root->MIN_SIZE = min_size;

    AddColliders(colliders);
}

Octree::~Octree() { delete root; }

void Octree::AddColliders(std::vector<Collider*> colliders) {
    for (auto collider : colliders) {
        root->AddCollider(collider);
    }
}

void Octree::Draw(std::shared_ptr<Shader> shader) {
    // glDisable(GL_DEPTH_TEST);
    root->Draw(shader);
    glEnable(GL_DEPTH_TEST);
}
}  // namespace gdp1
