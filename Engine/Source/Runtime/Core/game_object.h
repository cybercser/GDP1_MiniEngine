#pragma once

#include "Resource/level_object_description.h"
#include "transform.h"

namespace gdp1 {

class Scene;
class Model;
class Bounds;
class Animation;

class GameObject {
public:
    std::string name;
    std::string modelName;
    Transform* transform;
    Model* model;
    Scene* scene;
    bool visible;

    // temp variables for establishing hierarchy
    std::vector<std::string> childrenNames;
    std::string parentName;

public:
    GameObject() = delete;
    GameObject(Scene* scn, const GameObjectDesc& desc);
    GameObject(Scene* scn, const std::string& name);
    ~GameObject();

    const Bounds& GetBounds();
};

}  // namespace gdp1
