#pragma once

#include "Resource/level_object_description.h"
#include "transform.h"
#include "Physics/collision_info.h"

namespace gdp1 {

class Scene;
class Model;
class Bounds;
class Animation;
class SoftBody;

class GameObject {
public:
    std::string name;
    std::string modelName;
    Transform* transform;
    Model* model;
    Scene* scene;
    SoftBody* softBody;
    bool visible;
    bool hasSoftBody = false;
    bool hasFBO = false;

    // temp variables for establishing hierarchy
    std::vector<std::string> childrenNames;
    std::string parentName;

public:
    GameObject() = delete;
    GameObject(Scene* scn, const GameObjectDesc& desc);
    GameObject(Scene* scn, const std::string& name);
    ~GameObject();

    virtual void Update(float dt);
    virtual void OnCollision(CollisionInfo* collisionInfo);

    const Bounds& GetBounds();
};

}  // namespace gdp1
