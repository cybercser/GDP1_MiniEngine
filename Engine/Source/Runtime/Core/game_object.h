#pragma once

#include "Resource/level_object_description.h"
#include "transform.h"
#include "Physics/contact.h"
#include "Physics/rigidbody.h"
#include "Utils/fly_camera_controller.h"

namespace gdp1 {

class Scene;
class Model;
class Bounds;
class Animation;
class SoftBody;
class UniqueId;

class GameObject {
public:
    int id;
    std::string name;
    std::string modelName;
    Transform* transform;
    Model* model;
    Scene* scene;
    SoftBody* softBody;
    Rigidbody* rigidBody;


    bool visible;
    bool hasSoftBody = false;
    bool hasFBO = false;
    bool setLit = false;
    bool UseChromaticAberration = false;
    bool UseNightVision = false;
    int fboTextureId = 0;

    FlyCameraController* attachedCamera = nullptr;

    // temp variables for establishing hierarchy
    std::vector<std::string> childrenNames;
    std::string parentName;

public:
    GameObject() = delete;
    GameObject(Scene* scn, const GameObjectDesc& desc);
    GameObject(Scene* scn, const std::string& name);
    ~GameObject();

    virtual void Update(float dt);
    virtual void OnCollision(Contact* contact);
    virtual void OnEvent(Event& event);

    const Bounds& GetBounds();

    Bounds GetTransformedBounds();
};

}  // namespace gdp1
