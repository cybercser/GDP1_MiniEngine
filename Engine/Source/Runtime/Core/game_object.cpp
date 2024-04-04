#include "game_object.h"

#include "Physics/bounds.h"
#include "Render/model.h"
#include "Utils/unique_id_generator.h"
#include "Animation/character_animation.h"
#include "Utils/glm_utils.h"

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

void GameObject::SetCurrentAnimation(std::string name) {
    this->currentAnim = name;
    auto it = model->character_animations.find(name);
    if (it != model->character_animations.end()) {
        currentAnimation = it->second;
        blendFactor = 0.0f;
    } else {
        currentAnimation = nullptr;
    }

    // if (this->prevAnimation == nullptr) this->prevAnimation = this->currentAnimation;
}

void GameObject::UpdateAnimation(Shader* shader, float deltaTime) {
    if (currentAnimation) {
        elapsedAnimationTime += deltaTime;

        if (blendFactor < 1.0f) {
            blendFactor += deltaTime / blendDuration;
        }

        if (blendFactor > 1.0f) {
            blendFactor = 1.0f;
            prevAnimation = currentAnimation;
        }

        std::vector<aiMatrix4x4> transforms;

        uint32_t startAnimIndex = GetAnimationIndex(prevAnimation);
        uint32_t endAnimIndex = GetAnimationIndex(currentAnimation);

        if (startAnimIndex == -1) startAnimIndex = endAnimIndex;

        currentAnimation->boneTransformsBlended(elapsedAnimationTime, transforms, startAnimIndex, endAnimIndex,
                                                blendFactor);

        shader->SetUniform("u_HasBones", true);

        for (unsigned int i = 0; i < transforms.size(); i++) {
            std::string name = "bones[" + std::to_string(i) + "]";
            shader->SetUniform(name, GLMUtils::aiMatrix4x4ToGlmMat4(transforms[i]));
        }
    } else {
        shader->SetUniform("u_HasBones", false);
    }
    
    model->currentAnimation = currentAnimation;
    model->prevAnimation = prevAnimation;
}

uint32_t GameObject::GetAnimationIndex(CharacterAnimation* animation) {
    uint32_t index = 0;
    for (const auto& pair : model->character_animations) {
        if (pair.second == animation) {
            return index;
        }
        ++index;
    }

    return -1;
}

void GameObject::Update(float dt) {}

void GameObject::OnCollision(Contact* collisionInfo) {}

void GameObject::OnEvent(Event& event) {}

}  // namespace gdp1
