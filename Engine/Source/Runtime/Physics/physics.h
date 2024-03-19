#pragma once

#include <vector>
#include <string>
#include <map>

#include "Resource/level_loader.h"

namespace gdp1 {

// forward declaration
class Rigidbody;
class SoftBody;
struct Contact;
class Scene;
class Octree;
class Shader;

class Physics {
public:
    Physics(Scene* scene, const LevelDesc& levelDesc);
    ~Physics();

    void FixedUpdate(float deltaTime);

    bool AddImpulseToObject(const std::string& objectName, const glm::vec3& impulse);

    Rigidbody* FindRigidBodyByName(const std::string& name) const;

    SoftBody* FindSoftBodyByName(const std::string& name) const;

    void DrawBVH(std::shared_ptr<Shader> shader) const;

    void StartSoftBodyThreads();

private:
    void Init(Scene* scene, const LevelDesc& levelDesc);

    void CreateBVH();

private:
    Scene* scene;

    std::vector<Rigidbody*> rigidbodies_;
    std::vector<SoftBody*> softbodies_;

    std::map<std::string, Rigidbody*> body_map_;
    std::map<std::string, SoftBody*> soft_body_map_;

    std::unique_ptr<Octree> octree_;
};

}  // namespace gdp1
