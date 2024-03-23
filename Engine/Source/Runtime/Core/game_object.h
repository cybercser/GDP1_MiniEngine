#pragma once

#include "Resource/level_object_description.h"
#include "transform.h"
#include "Physics/collision_info.h"
#include "Utils/camera.h"

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
    bool setLit = false;
    bool UseChromaticAberration = false;
    bool UseNightVision = false;
    int fboTextureId = 0;

    Camera* fboCamera = nullptr;

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
