#include "game_object.h"

#include "Physics/bounds.h"
#include "Render/model.h"
#include "Utils/unique_id_generator.h"

namespace gdp1 {

GameObject::GameObject(Scene* scn, const GameObjectDesc& desc)
    : scene(scn)
    , model(nullptr)
    , name(desc.name)
    , modelName(desc.modelName)
    , visible(desc.visible)
    , hasFBO(desc.hasFBO)
    , setLit(desc.setLit) {
    id = UniqueId::GenerateId();
    transform = new Transform(this, desc.transform);
}

GameObject::GameObject(Scene* scn, const std::string& name)
    : scene(scn)
    , model(nullptr)
    , name(name)
    , modelName("")
    , visible(false)
    , hasFBO(false)
    , setLit(false) {
    id = UniqueId::GenerateId();
    transform = new Transform(this);
}

GameObject::~GameObject() {}

const Bounds& GameObject::GetBounds() {
    assert(model != nullptr);
    return model->bounds;
}

void GameObject::Update(float dt) {}

void GameObject::OnCollision(Contact* collisionInfo) {}

}  // namespace gdp1
