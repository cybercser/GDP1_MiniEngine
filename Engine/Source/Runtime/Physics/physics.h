#pragma once

#include <vector>
#include <string>
#include <map>

#include "Resource/level_loader.h"

namespace gdp1 {

// forward declaration
class Rigidbody;
struct Contact;
class Scene;
class Octree;
class Shader;

class Physics {
public:
    Physics(Scene* scene, const std::vector<RigidbodyDesc>& rigidbodyDescs);
    ~Physics();

    void FixedUpdate(float deltaTime);

    bool AddImpulseToObject(const std::string& objectName, const glm::vec3& impulse);

    Rigidbody* FindRigidbodyByName(const std::string& name) const;

    void DrawBVH(std::shared_ptr<Shader> shader) const;

private:
    void Init(Scene* scene, const std::vector<RigidbodyDesc>& rigidbodyDescs);

    void CreateBVH();

private:
    Scene* scene;

    std::vector<Rigidbody*> bodies_;
    std::map<std::string, Rigidbody*> body_map_;

    std::unique_ptr<Octree> octree_;
};

}  // namespace gdp1
