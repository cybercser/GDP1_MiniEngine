#include "scene.h"

#include "model.h"
#include "shader.h"
#include "light.h"
#include "skybox.h"
#include "Core/game_object.h"
#include "Core/application.h"
#include "Animation/animation_system.h"
#include "Utils/timer.h"
#include "Render/Buffers/ubo.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace glm;

DWORD WINAPI LoadModelThread(LPVOID lpParameter);

namespace gdp1 {

Scene::Scene(const LevelDesc& levelDesc) {
    this->levelDesc = levelDesc;
    CreateRootGameObject();
    ProcessDesc(levelDesc);
}

Scene::Scene(std::string levelFilePath) {
    LevelLoader loader;
    {
        GTimer timer = GTimer("LevelLoader");
        if (loader.LoadLevel(levelFilePath)) {
            LOG_INFO("Load scene successfully");
        } else {
            LOG_ERROR("Failed to load scene: {}", levelFilePath);
            return;
        }
    }

    levelDesc = loader.GetLevelDesc();
    CreateRootGameObject();
    ProcessDesc(levelDesc);
}

LevelDesc& Scene::GetLevelDesc() { return levelDesc; }

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

int Scene::DrawDebug(Shader* shader) {
    glDisable(GL_DEPTH_TEST);

    // draw debug
    for (std::unordered_map<std::string, GameObject*>::iterator it = m_GameObjectMap.begin();
         it != m_GameObjectMap.end(); it++) {
        GameObject* obj = it->second;
        if (obj != nullptr && obj->visible) {
            Model* model = obj->model;
            if (model != nullptr) {
                shader->Use();
                shader->SetUniform("u_Model", obj->transform->WorldMatrix());
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

void Scene::SetAnimationSpeed(float speed) { m_AnimationSystemPtr->SetPlaySpeed(speed); }

void Scene::SetAnimationPlaying(bool playing) { m_AnimationSystemPtr->SetPlaying(playing); }

void Scene::PlayNextAnimationClip() { m_AnimationSystemPtr->PlayNextClip(); }

void Scene::PlayPreviousAnimationClip() { m_AnimationSystemPtr->PlayPreviousClip(); }

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

void Scene::UpdateAnimation(float deltaTime) { m_AnimationSystemPtr->Update(deltaTime); }

void Scene::ProcessDesc(const LevelDesc& desc) {
    GTimer timer("ProcessDesc");
    LOG_INFO(desc.comment);

    LoadModels(desc.modelDescs);
    CreateGameObjects(desc.gameObjectDescs);
    CreateLights(desc.directionalLights, desc.pointLights, desc.spotLights);
    CreateSkybox(desc.skyboxDesc);
    LoadShaders(desc);
    CreateAnimations(desc.animationRefDesc);
    CreateCharacterAnimations(desc.characterAnimationRefDescs);
}

void Scene::LoadModel(std::unordered_map<std::string, Model*>* m_ModelMap, ModelDesc& modelDesc) {
    Model model = Model(modelDesc.filepath, modelDesc.shader, modelDesc.textures, 1, {});
    // std::lock_guard<std::mutex> lock(s_ModelsMutex);
    m_ModelMap->insert(std::make_pair(modelDesc.name, &model));

    m_VertexCount += model.GetVertexCount();
    m_TriangleCount += model.GetTriangleCount();
}

void Scene::LoadModels(const std::vector<ModelDesc>& modelDescs) {
    GTimer timer("LoadModels");
    double lastTime = glfwGetTime();

    // #pragma omp parallel for
    //     for (const ModelDesc& modelDesc : modelDescs) {
    //         auto it = m_ModelMap.find(modelDesc.name);
    //         if (it != m_ModelMap.end()) {
    //             // Model already exists, you can skip adding it or handle the case as needed
    //             // For example, if you want to skip adding the model, you can continue to the next iteration
    //             continue;
    //         }
    //
    //         m_Futures.push_back(std::async(std::launch::async, &Scene::LoadModel, this, &m_ModelMap, modelDesc));
    //     }
    // #pragma end

    // #pragma omp parallel for
    // for (const ModelDesc& modelDesc : modelDescs) {
    //     auto it = m_ModelMap.find(modelDesc.name);
    //     if (it != m_ModelMap.end()) {
    //         // Model already exists, you can skip adding it or handle the case as needed
    //         // For example, if you want to skip adding the model, you can continue to the next iteration
    //         continue;
    //     }

    //    Model* model = new Model(modelDesc.filepath, modelDesc.shader, modelDesc.textures, 1, {});
    //    m_ModelMap.insert(std::make_pair(modelDesc.name, model));

    //    model->SetupMeshes();

    //    m_VertexCount += model->GetVertexCount();
    //    m_TriangleCount += model->GetTriangleCount();
    //}
    // #pragma end

    // m_VertexCount = 0;
    // m_TriangleCount = 0;

    // std::mutex modelMapMutex;
    // GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

    //// Define a lambda function to be executed in each thread
    // auto processModel = [&](const ModelDesc& modelDesc) {
    //     auto it = m_ModelMap.find(modelDesc.name);
    //     if (it != m_ModelMap.end()) {
    //         // Model already exists, skip adding it
    //         return;
    //     }

    //    Model* model = new Model(modelDesc.filepath, modelDesc.shader, modelDesc.textures, 1, {});
    //    std::lock_guard<std::mutex> lock(modelMapMutex);
    //    m_ModelMap.insert(std::make_pair(modelDesc.name, model));

    //    // You might need to synchronize access to m_VertexCount and m_TriangleCount
    //    // if they are shared between threads to avoid data races.
    //    m_VertexCount += model->GetVertexCount();
    //    m_TriangleCount += model->GetTriangleCount();
    //};

    //// Create a vector of threads
    // std::vector<std::thread> threads;

    // for (const ModelDesc& modelDesc : modelDescs) {
    //     threads.emplace_back(processModel, modelDesc);
    // }

    //// Join all threads to wait for them to finish
    // std::for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });

    // Initialize counters for vertex and triangle counts
    int vertexCount = 0;
    int triangleCount = 0;

    for (const ModelDesc& modelDesc : modelDescs) {
        // Model* model = new Model(modelDesc.filepath, modelDesc.shader, modelDesc.textures);

        LoadModelThreadParams* params = new LoadModelThreadParams{modelDesc, m_ModelMap, vertexCount, triangleCount};

        DWORD threadId;
        HANDLE hThread = CreateThread(NULL, 0, LoadModelThread, (LPVOID)params, 0, &(threadId));
        if (hThread != NULL) {
            // Add the thread handle to the vector
            modelThreadHandles.push_back(hThread);
        }
    }

    WaitForMultipleObjects(modelThreadHandles.size(), modelThreadHandles.data(), TRUE, INFINITE);

    // Close the thread handles
    for (HANDLE hThread : modelThreadHandles) {
        CloseHandle(hThread);
    }

    for (auto it : m_ModelMap) {
        it.second->SetupMeshes();
        it.second->LoadTextures();
    }

    m_VertexCount = vertexCount;
    m_TriangleCount = triangleCount;

    double currentTime = glfwGetTime();

    LOG_ERROR("Loading Models in {0} : ", float(currentTime - lastTime));
}

bool Scene::LoadShaders(const LevelDesc& desc) {
    GTimer timer("LoadShaders");

    // Set Lights Data
    lightsData = new Lights();
    lightSettings = new LightSettings();

    DirectionalLight* directionLight = FindDirectionalLightByName("Sun");
    lightsData->dirLight.dir = directionLight->direction;
    lightsData->dirLight.color = directionLight->color;
    lightsData->dirLight.intensity = directionLight->intensity;

    int lightIndex = 0;
    for (std::unordered_map<std::string, PointLight*>::iterator it = m_PointLightMap.begin();
         it != m_PointLightMap.end(); it++, lightIndex++) {
        PointLight* pointLight = it->second;
        lightsData->pointLights[lightIndex].pos = pointLight->position;
        lightsData->pointLights[lightIndex].color = pointLight->color;
        lightsData->pointLights[lightIndex].intensity = pointLight->intensity;
        lightsData->pointLights[lightIndex].c = pointLight->constant;
        lightsData->pointLights[lightIndex].q = pointLight->quadratic;
        lightsData->pointLights[lightIndex].l = pointLight->linear;
    }

    lightSettings->numPointLights = m_PointLightMap.size();
    lightSettings->numSpotLights = m_SpotLightMap.size();
    lightSettings->useLights = true;
    lightSettings->useDirLight = true;
    lightSettings->usePointLights = false;
    lightSettings->useSpotLights = false;

    lightBuffer = new UBO(
        3, {newStruct({Type::VEC3, Type::VEC4, Type::SCALAR}),
            newArray(MAX_POINT_LIGHTS,
                     newStruct({Type::VEC3, Type::VEC4, Type::SCALAR, Type::SCALAR, Type::SCALAR, Type::SCALAR})),
            newArray(MAX_SPOT_LIGHTS, newStruct({Type::VEC3, Type::VEC3, Type::SCALAR, Type::SCALAR, Type::VEC3,
                                                 Type::VEC3, Type::VEC3, Type::SCALAR, Type::SCALAR, Type::SCALAR}))});

    lightSettingsBuffer = new UBO(5, {
                                         Type::SCALAR,
                                         Type::SCALAR,
                                         Type::SCALAR,
                                         Type::SCALAR,
                                         Type::SCALAR,
                                         Type::SCALAR,
                                         newStruct({Type::VEC3, Type::VEC4, Type::SCALAR}),
                                     });

    try {
        lit_shader_ptr_ = new Shader();
        lit_shader_ptr_->CompileShader("Assets/Shaders/lit.vert.glsl");
        lit_shader_ptr_->CompileShader("Assets/shaders/lit.frag.glsl");
        lit_shader_ptr_->Link();
        lit_shader_ptr_->Use();

        lit_shader_ptr_->SetUniform("u_Material.s", vec3(0.2f, 0.2f, 0.2f));
        lit_shader_ptr_->SetUniform("u_Material.shininess", 32.0f);
        lit_shader_ptr_->SetUniform("u_UseProjTex", false);
        lit_shader_ptr_->SetUniform("u_UsePointLights", true);
        lit_shader_ptr_->SetUniformBlock("LightBlock", lightBuffer->bindingPos);
        lit_shader_ptr_->SetUniformBlock("LightSettings", lightSettingsBuffer->bindingPos);

        m_ShaderMap.insert(std::make_pair("lit", lit_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    try {
        inst_shader_ptr_ = new Shader();
        inst_shader_ptr_->CompileShader("Assets/Shaders/inst.vert.glsl");
        inst_shader_ptr_->CompileShader("Assets/shaders/lit.frag.glsl");
        inst_shader_ptr_->Link();
        inst_shader_ptr_->Use();

        inst_shader_ptr_->SetUniform("u_Material.s", vec3(0.2f, 0.2f, 0.2f));
        inst_shader_ptr_->SetUniform("u_Material.shininess", 32.0f);
        inst_shader_ptr_->SetUniform("u_UseProjTex", false);
        inst_shader_ptr_->SetUniform("u_UsePointLights", true);
        inst_shader_ptr_->SetUniformBlock("LightBlock", lightBuffer->bindingPos);
        inst_shader_ptr_->SetUniformBlock("LightSettings", lightSettingsBuffer->bindingPos);

        m_ShaderMap.insert(std::make_pair("inst", inst_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    // debug shader
    try {
        debug_shader_ptr_ = new Shader();
        debug_shader_ptr_->CompileShader("Assets/Shaders/debug.vert.glsl");
        debug_shader_ptr_->CompileShader("Assets/shaders/debug.frag.glsl");
        debug_shader_ptr_->Link();
        debug_shader_ptr_->Use();

        debug_shader_ptr_->SetUniform("u_Color", vec4(0.0f, 1.0f, 0.0f, 1.0f));

        m_ShaderMap.insert(std::make_pair("debug", debug_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    // untextured shader
    try {
        untextured_shader_ptr_ = new Shader();
        untextured_shader_ptr_->CompileShader("Assets/Shaders/untextured.vert.glsl");
        untextured_shader_ptr_->CompileShader("Assets/shaders/untextured.frag.glsl");
        untextured_shader_ptr_->Link();
        untextured_shader_ptr_->Use();

        // directional light
        untextured_shader_ptr_->SetUniform("u_DirLight.a", vec3(0.05f, 0.05f, 0.05f));
        untextured_shader_ptr_->SetUniform("u_DirLight.d", vec3(0.6f, 0.6f, 0.6f));
        untextured_shader_ptr_->SetUniform("u_DirLight.s", vec3(0.5f, 0.5f, 0.5f));

        untextured_shader_ptr_->SetUniform("u_Material.a", vec3(0.2f, 0.2f, 0.2f));
        untextured_shader_ptr_->SetUniform("u_Material.d", vec3(0.8f, 0.8f, 0.8f));
        untextured_shader_ptr_->SetUniform("u_Material.s", vec3(0.2f, 0.2f, 0.2f));
        untextured_shader_ptr_->SetUniform("u_Material.shininess", 32.0f);

        m_ShaderMap.insert(std::make_pair("untextured", untextured_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    // unlit shader
    try {
        unlit_shader_ptr_ = new Shader();
        unlit_shader_ptr_->CompileShader("Assets/Shaders/unlit.vert.glsl");
        unlit_shader_ptr_->CompileShader("Assets/shaders/unlit.frag.glsl");
        unlit_shader_ptr_->Link();
        unlit_shader_ptr_->Use();

        m_ShaderMap.insert(std::make_pair("unlit", unlit_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    // skybox shader
    try {
        skybox_shader_ptr_ = new Shader();
        skybox_shader_ptr_->CompileShader("Assets/Shaders/skybox.vert.glsl");
        skybox_shader_ptr_->CompileShader("Assets/Shaders/skybox.frag.glsl");
        skybox_shader_ptr_->Link();
        skybox_shader_ptr_->Use();

        skybox_shader_ptr_->SetUniform("u_Skybox", 0);

        m_ShaderMap.insert(std::make_pair("skybox", skybox_shader_ptr_));
    } catch (GLSLProgramException& e) {
        LOG_ERROR("GLSLProgramException: {}", e.what());
        return false;
    }

    lightBuffer->generate();
    lightBuffer->bind();
    lightBuffer->initNullData(GL_STATIC_DRAW);
    lightBuffer->bindRange();

    lightBuffer->startWrite();

    lightBuffer->writeElement<glm::vec3>(&lightsData->dirLight.dir);
    lightBuffer->writeElement<glm::vec4>(&lightsData->dirLight.color);
    lightBuffer->writeElement<float>(&lightsData->dirLight.intensity);

    unsigned int i = 0;
    for (; i < lightSettings->numPointLights; i++) {
        lightBuffer->writeElement<glm::vec3>(&lightsData->pointLights[i].pos);
        lightBuffer->writeElement<glm::vec4>(&lightsData->pointLights[i].color);
        lightBuffer->writeElement<float>(&lightsData->pointLights[i].intensity);
        lightBuffer->writeElement<float>(&lightsData->pointLights[i].c);
        lightBuffer->writeElement<float>(&lightsData->pointLights[i].l);
        lightBuffer->writeElement<float>(&lightsData->pointLights[i].q);
    }
    lightBuffer->advanceArray(MAX_POINT_LIGHTS - i);

    for (i = 0; i < lightSettings->numSpotLights; i++) {
        lightBuffer->writeElement<glm::vec3>(&lightsData->spotLights[i].pos);
        lightBuffer->writeElement<glm::vec3>(&lightsData->spotLights[i].dir);
        lightBuffer->writeElement<float>(&lightsData->spotLights[i].cutoff);
        lightBuffer->writeElement<float>(&lightsData->spotLights[i].outerCutoff);
        lightBuffer->writeElement<glm::vec3>(&lightsData->spotLights[i].ambient);
        lightBuffer->writeElement<glm::vec3>(&lightsData->spotLights[i].diffuse);
        lightBuffer->writeElement<glm::vec3>(&lightsData->spotLights[i].specular);
        lightBuffer->writeElement<float>(&lightsData->spotLights[i].c);
        lightBuffer->writeElement<float>(&lightsData->spotLights[i].l);
        lightBuffer->writeElement<float>(&lightsData->spotLights[i].q);
    }
    lightBuffer->advanceArray(MAX_SPOT_LIGHTS - i);

    lightBuffer->clear();

    lightSettingsBuffer->generate();
    lightSettingsBuffer->bind();
    lightSettingsBuffer->initNullData(GL_STATIC_DRAW);
    lightSettingsBuffer->bindRange();

    lightSettingsBuffer->startWrite();

    lightSettingsBuffer->writeElement<int>(&lightSettings->numPointLights);
    lightSettingsBuffer->writeElement<int>(&lightSettings->numSpotLights);

    lightSettingsBuffer->writeElement<bool>(&lightSettings->useLights);
    lightSettingsBuffer->writeElement<bool>(&lightSettings->useDirLight);
    lightSettingsBuffer->writeElement<bool>(&lightSettings->usePointLights);
    lightSettingsBuffer->writeElement<bool>(&lightSettings->useSpotLights);

    lightSettingsBuffer->writeElement<glm::vec3>(&lightsData->dirLight.dir);
    lightSettingsBuffer->writeElement<glm::vec4>(&lightsData->dirLight.color);
    lightSettingsBuffer->writeElement<float>(&lightsData->dirLight.intensity);

    lightSettingsBuffer->clear();

    return true;
}

void Scene::CreateGameObjects(const std::vector<GameObjectDesc>& gameObjectDescs) {
    GTimer timer("CreateGameObjects");
#pragma omp parallel for
    for (const GameObjectDesc& goDesc : gameObjectDescs) {
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
#pragma end

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

void Scene::CreateLights(const std::vector<DirectionalLight>& directionalLights,
                         const std::vector<PointLight>& pointLights, const std::vector<SpotLight>& spotLights) {
    GTimer timer("CreateLights");
    // create directional lights
    for (const DirectionalLight& lightDesc : directionalLights) {
        m_DirectionalLightMap.emplace(lightDesc.name, new DirectionalLight(lightDesc));
    }

    // create point lights
    for (const PointLight& lightDesc : pointLights) {
        m_PointLightMap.emplace(lightDesc.name, new PointLight(lightDesc));
    }

    // create spot lights
    for (const SpotLight& lightDesc : spotLights) {
        m_SpotLightMap.emplace(lightDesc.name, new SpotLight(lightDesc));
    }
}

void Scene::CreateSkybox(const SkyboxDesc& skyboxDesc) {
    GTimer timer("CreateSkybox");
    const std::vector<std::string>& faces = skyboxDesc.faces;
    skybox_ptr_ = std::make_shared<Skybox>(faces, skyboxDesc.size);
}

void Scene::CreateAnimations(const AnimationRefDesc& animationRefDesc) {
    GTimer timer("CreateAnimations");
    AnimationLoader animLoader;

    Animation* anim = new Animation();
    animLoader.LoadAnimation(animationRefDesc.path, *anim);

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

void Scene::CreateCharacterAnimations(const std::vector<CharacterAnimationRefDesc>& desc) {
    GTimer timer("CreateCharacterAnimations");
    for (const CharacterAnimationRefDesc& anim : desc) {
        std::unordered_map<std::string, Model*>::iterator modelIt = m_ModelMap.find(anim.model);
        if (modelIt != m_ModelMap.end()) {
            CharacterAnimation* animation = new CharacterAnimation(anim.path, anim.name, modelIt->second);
            modelIt->second->AddCharacterAnimation(anim.name, anim.path);
            modelIt->second->SetCurrentAnimation(anim.name);
        }
    }
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

void Scene::AddGameObject(GameObject* go) {
    Transform* xform = go->transform;
    if (!go->modelName.empty()) {
        go->model = FindModelByName(go->modelName);
    }

    go->scene = this;

    m_GameObjectMap.insert(std::make_pair(go->name, go));

    // top level game object is a child of the root game object
    if (go->parentName.empty()) {
        xform->parent = xform->root = m_RootTransform;
        xform->SetWorldMatrix(xform->LocalMatrix());
        m_RootTransform->children.push_back(xform);
    }

    UpdateHierarchy(xform);
}

void Scene::AddPointLight(PointLight& pointLight) {
    m_PointLightMap.emplace(pointLight.name, new PointLight(pointLight));
}

DirectionalLight* Scene::FindDirectionalLightByName(const std::string& name) {
    DirectionalLightMap::iterator it = m_DirectionalLightMap.find(name);
    if (it != m_DirectionalLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

PointLight* Scene::FindPointLightByName(const std::string& name) {
    PointLightMap::iterator it = m_PointLightMap.find(name);
    if (it != m_PointLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

SpotLight* Scene::FindSpotLightByName(const std::string& name) {
    SpotLightMap::iterator it = m_SpotLightMap.find(name);
    if (it != m_SpotLightMap.end()) {
        return it->second;
    }
    return nullptr;
}

size_t Scene::GetAnimationCurClipIndex() const { return m_AnimationSystemPtr->GetCurrentClipIndex(); }

float Scene::GetAnimationSpeed() const { return m_AnimationSystemPtr->GetSpeed(); }

float Scene::GetAnimationElapsedTime() const { return m_AnimationSystemPtr->GetElapsedTime(); }

void Scene::CreateFBO() { fbo_ptr_ = std::make_shared<FBO>(512, 512); }

void Scene::UseFBO() { fbo_ptr_.get()->Bind(); }

bool Scene::HasFBO() { return fbo_ptr_.get() != nullptr; }

FBO* Scene::GetFBO() { return fbo_ptr_.get(); }

}  // namespace gdp1