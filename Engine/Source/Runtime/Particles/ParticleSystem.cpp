#include "ParticleSystem.h"

#include <random>
#include <Render/scene.h>
#include <Render/model.h>
#include <Core/game_object.h>
#include <Resource/level_object_description.h>

namespace gdp1 {

float RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

ParticleSystem::ParticleSystem(std::shared_ptr<Scene> scene, unsigned int numParticles) {
    this->numParticles = numParticles;
    this->scene = scene;

    for (int i = 0; i < numParticles; ++i) {
        // Generate random position, size, and other properties
        float x = RandomFloat(-7.0f, 7.0f);      // Random X position
        float y = RandomFloat(0.0f, 4.0f);       // Random Y position (height)
        float z = RandomFloat(-7.0f, 7.0f);      // Random Z position
        float scale = RandomFloat(0.05f, 0.2f);  // Random scale
        bool isVisible = true;                   // Raindrop is initially visible

        glm::vec4 randomColor(RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), 1.0f);

        // Create a GameObjectDesc for the raindrop
        GameObjectDesc desc;
        desc.name = "Raindrop" + std::to_string(i);
        desc.modelName = "sphere";
        desc.visible = isVisible;
        desc.hasFBO = false;
        desc.transform.localPosition = {x, y, z};
        desc.transform.localScale = {scale, scale, scale};
        desc.setLit = true;
        desc.isStatic = true;

        // Create the raindrop GameObject and add it to the scene
        GameObject* particle = new GameObject(scene.get(), desc);

        /*RigidbodyDesc rigidbodyDesc;
        rigidbodyDesc.active = true;
        rigidbodyDesc.collider = "SPHERE";
        rigidbodyDesc.invMass = 1.0f;
        rigidbodyDesc.objectName = desc.name;
        rigidbodyDesc.orientation = glm::vec3(0.0f);
        rigidbodyDesc.position = glm::vec3(0.0f);
        rigidbodyDesc.velocity = glm::vec3(0.0f);

        PointLight pointLight;
        pointLight.color = randomColor;
        pointLight.name = desc.name;
        pointLight.position = {x, y, z};
        pointLight.intensity = 0.7f;
        pointLight.constant = 0.5f;
        pointLight.linear = 2.0f;
        pointLight.quadratic = 1.0f;

        scene->AddPointLight(pointLight);
        m_Scene->GetLevelDesc().rigidbodyDescs.push_back(rigidbodyDesc);*/

        //particles.push_back(particle);
        instanceMatrices.push_back(particle->transform->LocalMatrix());
    }

    particleModel = scene->FindModelByName("sphere");
    particleModel->SetupInstancing(instanceMatrices);
}

void ParticleSystem::Render() {
    particleModel->Draw(scene->inst_shader_ptr_);
}

ParticleSystem::~ParticleSystem() {

}

}  // namespace gdp1