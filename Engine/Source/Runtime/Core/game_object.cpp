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

Bounds GameObject::GetTransformedBounds() {
    Bounds originalBounds = GetBounds();
    glm::mat4 worldTransform = transform->WorldMatrix();

    // Transform the original bounds using the world transform matrix
    glm::vec4 transformedMin = worldTransform * glm::vec4(originalBounds.GetMin(), 1.0f);
    glm::vec4 transformedMax = worldTransform * glm::vec4(originalBounds.GetMax(), 1.0f);

    // Extract the transformed minimum and maximum corners from the transformed vectors
    Bounds transformedBounds;
    transformedBounds.SetMinMax(glm::vec3(transformedMin), glm::vec3(transformedMax));

    return transformedBounds;
}

void GameObject::Update(float dt) {}

void GameObject::OnCollision(Contact* collisionInfo) {}

void GameObject::OnEvent(Event& event) {}

}  // namespace gdp1
