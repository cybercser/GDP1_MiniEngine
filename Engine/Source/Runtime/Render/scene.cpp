#include "scene.h"

#include "model.h"
#include "shader.h"
#include "light.h"
#include "skybox.h"
#include "Core/game_object.h"
#include "Animation/animation_system.h"

using namespace glm;

namespace gdp1 {

Scene::Scene(const LevelDesc& levelDesc) {
    CreateRootGameObject();
    ProcessDesc(levelDesc);
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

    UpdateHierarchyWorldMatrix();
}

void Scene::UpdateHierarchyWorldMatrix() {
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
    LOG_INFO(desc.comment);

    LoadModels(desc.modelDescs);
    CreateGameObjects(desc.gameObjectDescs);
    CreateLights(desc.directionalLights, desc.pointLights, desc.spotLights);
    CreateSkybox(desc.skyboxDesc);
    LoadShaders(desc);
    CreateAnimations(desc.animationRefDesc);
}

void Scene::LoadModels(const std::vector<ModelDesc>& modelDescs) {
    for (const ModelDesc& modelDesc : modelDescs) {
        Model* model = new Model(modelDesc.filepath, modelDesc.shader);
        m_ModelMap.insert(std::make_pair(modelDesc.name, model));

        m_VertexCount += model->GetVertexCount();
        m_TriangleCount += model->GetTriangleCount();
    }
}

bool Scene::LoadShaders(const LevelDesc& desc) {
    try {
        lit_shader_ptr_ = new Shader();
        lit_shader_ptr_->CompileShader("Assets/Shaders/lit.vert.glsl");
        lit_shader_ptr_->CompileShader("Assets/shaders/lit.frag.glsl");
        lit_shader_ptr_->Link();
        lit_shader_ptr_->Use();

        DirectionalLight* dirLight = FindDirectionalLightByName("Sun");
        if (dirLight == nullptr) {
            LOG_ERROR("Cannot find directional light: Sun");
            return false;
        }

        // directional light
        lit_shader_ptr_->SetUniform("u_DirLight.color", dirLight->color);
        lit_shader_ptr_->SetUniform("u_DirLight.intensity", dirLight->intensity);

        lit_shader_ptr_->SetUniform("u_Material.s", vec3(0.2f, 0.2f, 0.2f));
        lit_shader_ptr_->SetUniform("u_Material.shininess", 32.0f);

        lit_shader_ptr_->SetUniform("u_UseProjTex", false);

        m_ShaderMap.insert(std::make_pair("lit", lit_shader_ptr_));
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

    return true;
}

void Scene::CreateGameObjects(const std::vector<GameObjectDesc>& gameObjectDescs) {
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

    // establish hierarchy
    for (Transform* xform : m_RootTransform->children) {
        CreateHierarchy(xform);
    }

    // The first time we update all the game objects' world matrices
    UpdateHierarchyWorldMatrix();
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
    const std::vector<std::string>& faces = skyboxDesc.faces;
    skybox_ptr_ = std::make_shared<Skybox>(faces, skyboxDesc.size);
}

void Scene::CreateAnimations(const AnimationRefDesc& animationRefDesc) {
    AnimationLoader animLoader;
    LOG_INFO("loading animation {}", animationRefDesc.name);

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

}  // namespace gdp1