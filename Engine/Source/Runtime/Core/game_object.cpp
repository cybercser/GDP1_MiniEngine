#include "game_object.h"

#include "Physics/bounds.h"
#include "Render/model.h"

namespace gdp1 {

GameObject::GameObject(Scene* scn, const GameObjectDesc& desc)
    : scene(scn)
    , model(nullptr)
    , name(desc.name)
    , modelName(desc.modelName)
    , visible(desc.visible)
    , hasFBO(desc.hasFBO) {
    transform = new Transform(this, desc.transform);
}

GameObject::GameObject(Scene* scn, const std::string& name)
    : scene(scn)
    , model(nullptr)
    , name(name)
    , modelName("")
    , visible(false)
    , hasFBO(false) {
    transform = new Transform(this);
}

GameObject::~GameObject() {}

const Bounds& GameObject::GetBounds() {
    assert(model != nullptr);
    return model->bounds;
}

void GameObject::Update(float dt) {}

void GameObject::OnCollision(CollisionInfo* collisionInfo) {}

}  // namespace gdp1
