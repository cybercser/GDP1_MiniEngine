#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Resource/level_object_descriptor.h"

namespace gdp1 {

// forward declaration
class Rigidbody;
struct Contact;
class Scene;
class Octree;

class Physics {
public:
    Physics(Scene* scene, const std::vector<RigidbodyDesc>& rigidbodyDescs);
    ~Physics();

    void FixedUpdate(float deltaTime);

    bool AddImpulseToObject(const std::string& objectName, const glm::vec3& impulse);

    Rigidbody* FindRigidbodyByName(const std::string& name) const;

private:
    void Init(const std::vector<RigidbodyDesc>& rigidbodyDescs);

    void CreateOctree();

private:
    Scene* m_Scene;

    std::vector<Rigidbody*> m_Rigidbodies;
    std::unordered_map<std::string, Rigidbody*> m_RigidbodyMap;

    std::unique_ptr<Octree> m_Octree;
};

}  // namespace gdp1
