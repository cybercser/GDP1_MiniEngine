#pragma once

#include "common.h"
#include "shader.h"
#include "light.h"
#include "fbo.h"
#include "Resource/level_loader.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

namespace gdp1 {

// forward declaration
class Model;
class GameObject;
class Shader;
class Skybox;
class Animation;
class AnimationSystem;
class Renderer;

struct LoadModelThreadParams {
    ModelDesc modelDesc;
    std::unordered_map<std::string, Model*>& modelMap;
    int& vertexCount;
    int& triangleCount;
};

class Scene {
public:

    Scene(const LevelDesc& levelJson);
    Scene(std::string levelFilePath);
    ~Scene();
    void CreateRootGameObject();

    int DrawDebug(Shader* debugShader);

    void Update(float deltaTime);

    // animation control
    void SetAnimationSpeed(float speed);
    void SetAnimationPlaying(bool playing);
    void PlayNextAnimationClip();
    void PlayPreviousAnimationClip();

    // helpers
    Model* FindModelByName(const std::string& name);
    GameObject* FindObjectByName(const std::string& name);

    void AddGameObject(GameObject* gameObject);
    void AddPointLight(PointLight& pointLight);

    DirectionalLight* FindDirectionalLightByName(const std::string& name);
    PointLight* FindPointLightByName(const std::string& name);
    SpotLight* FindSpotLightByName(const std::string& name);

    unsigned int GetVertexCount() const { return m_VertexCount; }
    unsigned int GetTriangleCount() const { return m_TriangleCount; }

    size_t GetAnimationCurClipIndex() const;
    float GetAnimationSpeed() const;
    float GetAnimationElapsedTime() const;

    LevelDesc& GetLevelDesc();

    std::unordered_map<std::string, GameObject*> GetGameObjectMap() { return m_GameObjectMap; };

    void CreateFBO();
    void UseFBO();
    bool HasFBO();
    FBO* GetFBO();

private:
    void UpdateAnimation(float deltaTime);
    void UpdateHierarchy(Transform* xform);

    void ProcessDesc(const LevelDesc& desc);
    void LoadModels(const std::vector<ModelDesc>& desc);
    bool LoadShaders(const LevelDesc& desc);
    void CreateGameObjects(const std::vector<GameObjectDesc>& desc);
    void CreateLights(const std::vector<DirectionalLight>& directionalLights,
                      const std::vector<PointLight>& pointLights, const std::vector<SpotLight>& spotLights);
    void CreateSkybox(const SkyboxDesc& skyboxDesc);
    void CreateAnimations(const AnimationRefDesc& animationRefDesc);
    void CreateCharacterAnimations(const std::vector<CharacterAnimationRefDesc>& desc);

    void CreateHierarchy(Transform* xform);

private:
    std::vector<HANDLE> modelThreadHandles;

    std::unordered_map<std::string, Model*> m_ModelMap;
    std::unordered_map<std::string, GameObject*> m_GameObjectMap;

    // #TODO: we should have a better way (material system) to manage shaders
    Shader* lit_shader_ptr_;
    Shader* debug_shader_ptr_;
    Shader* untextured_shader_ptr_;
    Shader* unlit_shader_ptr_;
    Shader* skybox_shader_ptr_;
    std::unordered_map<std::string, Shader*> m_ShaderMap;

    using DirectionalLightMap = std::unordered_map<std::string, DirectionalLight*>;
    using PointLightMap = std::unordered_map<std::string, PointLight*>;
    using SpotLightMap = std::unordered_map<std::string, SpotLight*>;

    DirectionalLightMap m_DirectionalLightMap;
    PointLightMap m_PointLightMap;
    SpotLightMap m_SpotLightMap;

    std::shared_ptr<Skybox> skybox_ptr_;
    std::shared_ptr<FBO> fbo_ptr_;

    Transform* m_RootTransform;
    GameObject* m_RootGameObject;  // root transform must belong to a game object

    std::unique_ptr<AnimationSystem> m_AnimationSystemPtr;

    LevelDesc levelDesc;

    unsigned int m_VertexCount;
    unsigned int m_TriangleCount;

    friend class Renderer;
};

}  // namespace gdp1
