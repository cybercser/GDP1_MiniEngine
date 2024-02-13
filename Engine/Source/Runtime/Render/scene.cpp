#include "scene.h"

#include <iostream>
#include <memory>
#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "model.h"
#include "Core/game_object.h"
#include "Animation/animation_system.h"

namespace gdp1 {

Scene::Scene(const LevelDesc& levelDesc) {
    CreateRootGameObject();
    Init(levelDesc);
}

void Scene::CreateRootGameObject() {
    m_RootGameObject = new GameObject(this, "~Root");
    m_RootGameObject->visible = false;

    m_RootTransform = new Transform(m_RootGameObject);
    m_RootTransform->root = m_RootTransform;
    m_RootTransform->parent = nullptr;
    m_RootTransform->UpdateLocalMatrix();

    m_RootGameObject->transform = m_RootTransform;
}

Scene::~Scene() {
    // Cleanup models and gameObjects
    for (std::unordered_map<std::string, Model*>::iterator it = m_ModelMap.begin(); it != m_ModelMap.end(); it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
    for (std::unordered_map<std::string, GameObject*>::iterator it = m_GameObjectMap.begin();
         it != m_GameObjectMap.end(); it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
    // Cleanup lights
    for (std::unordered_map<std::string, DirectionalLight*>::iterator it = m_DirectionalLightMap.begin();
         it != m_DirectionalLightMap.end(); it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
    for (std::unordered_map<std::string, PointLight*>::iterator it = m_PointLightMap.begin();
         it != m_PointLightMap.end(); it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
    for (std::unordered_map<std::string, SpotLight*>::iterator it = m_SpotLightMap.begin(); it != m_SpotLightMap.end();
         it++) {
        if (it->second != nullptr) {
            delete it->second;
        }
    }
}

void Scene::Init(const LevelDesc& desc) {
    ProcessDesc(desc);
}

int Scene::Draw(const std::unordered_map<std::string, std::shared_ptr<Shader>>& shaderMap) {
    for (std::unordered_map<std::string, GameObject*>::iterator it = m_GameObjectMap.begin();
         it != m_GameObjectMap.end(); it++) {
        GameObject* obj = it->second;
        if (obj != nullptr && obj->visible) {
            Model* model = obj->model;
            if (model != nullptr) {
                // find shader
                std::unordered_map<std::string, std::shared_ptr<Shader>>::const_iterator shaderIt =
                    shaderMap.find(model->shaderName);
                if (shaderIt == shaderMap.end()) {
                    LOG_ERROR("Cannot find shader: " + model->shaderName);
                    continue;
                }
                std::shared_ptr<Shader> shader = shaderIt->second;
                shader->use();
                shader->setUniform("u_Model", obj->transform->WorldMatrix());
                model->Draw(shader);
            }
        }
    }

    return 0;
}

int Scene::DrawDebug(std::shared_ptr<Shader> shader) {
    glDisable(GL_DEPTH_TEST);

    // draw debug
    for (std::unordered_map<std::string, GameObject*>::iterator it = m_GameObjectMap.begin();
         it != m_GameObjectMap.end(); it++) {
        GameObject* obj = it->second;
        if (obj != nullptr && obj->visible) {
            Model* model = obj->model;
            if (model != nullptr) {
                shader->use();
                shader->setUniform("u_Model", obj->transform->WorldMatrix());
                model->DrawDebug(shader);
            }
        }
    }

    glEnable(GL_DEPTH_TEST);

    return 0;
}

void Scene::Update(float deltaTime) {
    UpdateAnimation(deltaTime);

    for (Transform* xform : m_RootTransform->children) {
        UpdateHierarchy(xform);
    }
}

void Scene::SetAnimationSpeed(float speed) {
    m_AnimationSystemPtr->SetPlaySpeed(speed);
}

void Scene::SetAnimationPlaying(bool playing) {
    m_AnimationSystemPtr->SetPlaying(playing);
}

void Scene::PlayNextAnimationClip() {
    m_AnimationSystemPtr->PlayNextClip();
}

void Scene::PlayPreviousAnimationClip() {
    m_AnimationSystemPtr->PlayPreviousClip();
}

void Scene::UpdateHierarchy(Transform* xform) {
    if (xform->hasChanged) {
        xform->UpdateLocalMatrix();
    }

    for (Transform* child : xform->children) {
        child->hasChanged = true;
        UpdateHierarchy(child);
    }

    if (xform->parent == m_RootTransform) {
        xform->SetWorldMatrix(xform->LocalMatrix());
    } else {
        xform->SetWorldMatrix(xform->parent->WorldMatrix() * xform->LocalMatrix());
    }

    xform->hasChanged = false;
}

void Scene::UpdateAnimation(float deltaTime) {
    m_AnimationSystemPtr->Update(deltaTime);
}

int Scene::ProcessDesc(const LevelDesc& desc) {
    LOG_INFO(desc.comment);

    LoadModels(desc);
    CreateGameObjects(desc);
    CreateLights(desc);
    CreateAnimations(desc);

    return 0;
}

int Scene::LoadModels(const LevelDesc& desc) {
    for (const ModelDesc& modelDesc : desc.modelDescs) {
        Model* model = new Model(modelDesc.filepath, modelDesc.shader);
        m_ModelMap.insert(std::make_pair(modelDesc.name, model));

        m_VertexCount += model->GetVertexCount();
        m_TriangleCount += model->GetTriangleCount();
    }
    return 0;
}

void Scene::CreateGameObjects(const LevelDesc& desc) {
    for (const GameObjectDesc& goDesc : desc.gameObjectDescs) {
        GameObject* go = new GameObject(this, goDesc);
        Transform* xform = go->transform;
        if (!go->modelName.empty()) {
            go->model = FindModelByName(go->modelName);
        }
        go->scene = this;
        m_GameObjectMap.insert(std::make_pair(go->name, go));

        go->childrenNames = goDesc.children;
        go->parentName = goDesc.parentName;

        // top level game object is a child of the root game object
        if (go->parentName.empty()) {
            xform->parent = xform->root = m_RootTransform;
            xform->SetWorldMatrix(xform->LocalMatrix());
            m_RootTransform->children.push_back(xform);
        }
    }

    // establish hierarchy
    for (Transform* xform : m_RootTransform->children) {
        CreateHierarchy(xform);
    }
}

void Scene::CreateHierarchy(Transform* xform) {
    GameObject* go = xform->gameObject;
    for (const std::string& childName : go->childrenNames) {
        std::unordered_map<std::string, GameObject*>::iterator childGOIt = m_GameObjectMap.find(childName);
        if (childGOIt != m_GameObjectMap.end()) {
            GameObject* child = childGOIt->second;
            Transform* childXform = child->transform;

            xform->children.push_back(childXform);
            childXform->parent = xform;
            childXform->root = m_RootTransform;

            childXform->SetWorldMatrix(xform->WorldMatrix() * childXform->LocalMatrix());

            CreateHierarchy(childXform);
        }
    }
}

void Scene::CreateLights(const LevelDesc& desc) {
    // create directional lights
    for (const DirectionalLight& lightDesc : desc.directionalLights) {
        DirectionalLight* light = new DirectionalLight();
        light->name = lightDesc.name;
        light->direction = lightDesc.direction;

        light->color = lightDesc.color;
        light->intensity = lightDesc.intensity;

        m_DirectionalLightMap.insert(std::make_pair(light->name, light));
    }

    // create point lights
    for (const PointLight& lightDesc : desc.pointLights) {
        PointLight* light = new PointLight();
        light->name = lightDesc.name;
        light->position = lightDesc.position;

        light->color = lightDesc.color;
        light->intensity = lightDesc.intensity;

        light->constant = lightDesc.constant;
        light->linear = lightDesc.linear;
        light->quadratic = lightDesc.quadratic;

        m_PointLightMap.insert(std::make_pair(light->name, light));
    }

    // create spot lights
    for (const SpotLight& lightDesc : desc.spotLights) {
        SpotLight* light = new SpotLight();
        light->name = lightDesc.name;
        light->position = lightDesc.position;
        light->direction = lightDesc.direction;
        light->cutoff = lightDesc.cutoff;
        light->outerCutoff = lightDesc.outerCutoff;

        light->ambient = lightDesc.ambient;
        light->diffuse = lightDesc.diffuse;
        light->specular = lightDesc.specular;

        light->constant = lightDesc.constant;
        light->linear = lightDesc.linear;
        light->quadratic = lightDesc.quadratic;

        light->cookie = lightDesc.cookie;

        m_SpotLightMap.insert(std::make_pair(light->name, light));
    }
}

void Scene::CreateAnimations(const LevelDesc& desc) {
    AnimationLoader animLoader;
    LOG_INFO("loading animation {}", desc.animationRefDesc.name);

    Animation* anim = new Animation();
    animLoader.LoadAnimation(desc.animationRefDesc.path, *anim);

    // set the GameObject for each keyframe
    for (AnimationClip& clip : anim->clips) {
        // for each position keyframe
        for (PositionKeyframe& keyframe : clip.positionKeys) {
            GameObject* go = FindObjectByName(keyframe.objectName);
            if (go != nullptr) {
                keyframe.xform = go->transform;
            }
        }
        // for each rotation keyframe
        for (RotationKeyframe& keyframe : clip.rotationKeys) {
            GameObject* go = FindObjectByName(keyframe.objectName);
            if (go != nullptr) {
                keyframe.xform = go->transform;
            }
        }
        // for each lossyScale keyframe
        for (ScaleKeyframe& keyframe : clip.scaleKeys) {
            GameObject* go = FindObjectByName(keyframe.objectName);
            if (go != nullptr) {
                keyframe.xform = go->transform;
            }
        }
    }

    m_AnimationSystemPtr = std::make_unique<AnimationSystem>(anim);
}

Model* Scene::FindModelByName(const std::string& name) {
    std::unordered_map<std::string, Model*>::iterator it = m_ModelMap.find(name);
    if (it != m_ModelMap.end()) {
        return it->second;
    }
    return nullptr;
}

GameObject* Scene::FindObjectByName(const std::string& name) {
    std::unordered_map<std::string, GameObject*>::iterator it = m_GameObjectMap.find(name);
    if (it != m_GameObjectMap.end()) {
        return it->second;
    }
    return nullptr;
}

DirectionalLight* Scene::FindDirectionalLightByName(const std::string& name) {
    std::unordered_map<std::string, DirectionalLight*>::iterator it = m_DirectionalLightMap.find(name);
    if (it != m_DirectionalLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

PointLight* Scene::FindPointLightByName(const std::string& name) {
    std::unordered_map<std::string, PointLight*>::iterator it = m_PointLightMap.find(name);
    if (it != m_PointLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

SpotLight* Scene::FindSpotLightByName(const std::string& name) {
    std::unordered_map<std::string, SpotLight*>::iterator it = m_SpotLightMap.find(name);
    if (it != m_SpotLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

size_t Scene::GetAnimationCurClipIndex() const {
    return m_AnimationSystemPtr->GetCurrentClipIndex();
}

float Scene::GetAnimationSpeed() const {
    return m_AnimationSystemPtr->GetSpeed();
}

float Scene::GetAnimationElapsedTime() const {
    return m_AnimationSystemPtr->GetElapsedTime();
}

}  // namespace gdp1