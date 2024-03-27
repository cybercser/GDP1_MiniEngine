#include "collectible.h"

Collectible::Collectible(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc, CollectibleType collectible_type)
    : gdp1::GameObject(scn, desc)
    , collectible_type(collectible_type) {}

Collectible::Collectible(gdp1::Scene* scn, const std::string& name, CollectibleType collectible_type)
    : gdp1::GameObject(scn, name)
    , collectible_type(collectible_type) {}

void Collectible::Update(float dt) {
    if (collectible_type == COIN) {
        float rotationAngle = 1.0 * dt;  // You may need to define rotationSpeed

        // Create a rotation quaternion around the y-axis
        glm::quat rotation = glm::angleAxis(rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply the rotation to the object's transform
        transform->SetRotation(rotation * transform->localRotation);
    }
}

void Collectible::OnCollision(gdp1::Contact* contact) {
    if (contact->bodyB->object->name == "Player" || contact->bodyA->object->name == "Player") {
        this->rigidBody->active = false;
        this->visible = false;
    }
}