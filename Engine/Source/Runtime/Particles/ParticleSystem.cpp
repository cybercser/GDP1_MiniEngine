#include "ParticleSystem.h"

#include <random>
#include <Render/scene.h>
#include <Render/model.h>
#include <Render/frustum.h>
#include <Core/game_object.h>
#include <Utils/camera.h>
#include <Resource/level_object_description.h>

using namespace glm;
using namespace std;

namespace gdp1 {

float RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

ParticleSystem::ParticleSystem(std::shared_ptr<Scene> scene, unsigned int numParticles) {
    this->numParticles = numParticles;
    this->scene = scene;

    bounds = new Bounds();
    particleModel = scene->FindModelByName("Cube");

    if (particleModel == nullptr) return;

#pragma omp parallel for
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
        desc.name = "sphere" + std::to_string(i);
        desc.modelName = "sphere";
        desc.visible = isVisible;
        desc.hasFBO = false;
        desc.transform.localPosition = {x, y, z};
        desc.transform.localScale = {scale, scale, scale};
        desc.setLit = true;
        desc.isStatic = true;

        // Create the raindrop GameObject and add it to the scene
        GameObject* particle = new GameObject(scene.get(), desc);
        particle->model = particleModel;

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

        PointLight pointLight;
        pointLight.color = randomColor;
        pointLight.name = desc.name;
        pointLight.position = {x, y, z};
        pointLight.intensity = 0.7f;
        pointLight.constant = 0.5f;
        pointLight.linear = 2.0f;
        pointLight.quadratic = 1.0f;

        scene->AddPointLight(pointLight);

        // particles.push_back(particle);
        particles[desc.name] = particle;
        particle->transform->SetWorldMatrix(particle->transform->LocalMatrix());
        particleModel->bounds.TransformBounds(particle->transform->WorldMatrix());
        instanceMatrices.push_back(particle->transform->WorldMatrix());

        bounds->Expand(particle->model->bounds);
    }

    particleModel->SetupInstancing(instanceMatrices, false);
}

void ParticleSystem::Render(std::shared_ptr<Camera> camera) {
    mat4 projection = camera->GetProjectionMatrix();
    mat4 view = camera->GetViewMatrix();
    mat4 umodel = mat4(1.0f);
    mat4 mv = view * umodel;
    mat3 normalMatrix = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

    std::unordered_map<std::string, GameObject*> culledParticles;

    Frustum viewFrustum;
    viewFrustum.Update(projection * view);

    if (!viewFrustum.IsBoxInFrustum(bounds->GetMin(), bounds->GetMax())) {
        return;
    }

    /*culledParticles = viewFrustum.GetCulledObjects(particles);

    instanceMatrices.clear();
    for (std::unordered_map<std::string, GameObject*>::iterator it = particles.begin(); it != particles.end(); it++) {
        instanceMatrices.push_back(it->second->transform->WorldMatrix());
    }

    particleModel->ResetInstancing();
    particleModel->SetupInstancing(instanceMatrices);*/

    DirectionalLight* dirLight = scene->FindDirectionalLightByName("Sun");

    vec3 lightDirViewSpace = normalMatrix * dirLight->direction;

    scene->inst_shader_ptr_->Use();
    scene->inst_shader_ptr_->SetUniform("u_View", view);
    scene->inst_shader_ptr_->SetUniform("u_Proj", projection);
    scene->inst_shader_ptr_->SetUniform("u_NormalMat", normalMatrix);

    scene->inst_shader_ptr_->SetUniform("u_DirLight.dir", lightDirViewSpace);
    scene->inst_shader_ptr_->SetUniform("u_NumPointLights", (int)scene->m_PointLightMap.size());
    scene->inst_shader_ptr_->SetUniform("u_UseLights", true);

    particleModel->Draw(scene->inst_shader_ptr_);
}

ParticleSystem::~ParticleSystem() {}

}  // namespace gdp1