#pragma once

#include "common.h"
#include "shader.h"
#include "light.h"
#include "Resource/level_loader.h"

namespace gdp1 {

// forward declaration
class Model;
class GameObject;
class Animation;
class AnimationSystem;

class Scene {
public:
    Scene(const LevelDesc& levelJson);

    void CreateRootGameObject();

    ~Scene();

    int Draw(const std::unordered_map<std::string, std::shared_ptr<Shader>>& shaderMap);

    int DrawDebug(std::shared_ptr<Shader> debugShader);

    void Update(float deltaTime);

    // animation control
    void SetAnimationSpeed(float speed);
    void SetAnimationPlaying(bool playing);
    void PlayNextAnimationClip();
    void PlayPreviousAnimationClip();

    // helpers
    Model* FindModelByName(const std::string& name);
    GameObject* FindObjectByName(const std::string& name);

    DirectionalLight* FindDirectionalLightByName(const std::string& name);
    PointLight* FindPointLightByName(const std::string& name);
    SpotLight* FindSpotLightByName(const std::string& name);

    unsigned int GetVertexCount() const {
        return m_VertexCount;
    }
    unsigned int GetTriangleCount() const {
        return m_TriangleCount;
    }

    size_t GetAnimationCurClipIndex() const;
    float GetAnimationSpeed() const;
    float GetAnimationElapsedTime() const;

private:
    void UpdateAnimation(float deltaTime);
    void UpdateHierarchy(Transform* xform);

    void Init(const LevelDesc& desc);
    int ProcessDesc(const LevelDesc& desc);
    int LoadModels(const LevelDesc& desc);
    void CreateGameObjects(const LevelDesc& desc);
    void CreateLights(const LevelDesc& desc);
    void CreateAnimations(const LevelDesc& desc);

    void CreateHierarchy(Transform* xform);

private:
    std::unordered_map<std::string, Model*> m_ModelMap;
    std::unordered_map<std::string, GameObject*> m_GameObjectMap;

    std::unordered_map<std::string, DirectionalLight*> m_DirectionalLightMap;
    std::unordered_map<std::string, PointLight*> m_PointLightMap;
    std::unordered_map<std::string, SpotLight*> m_SpotLightMap;

    Transform* m_RootTransform;
    GameObject* m_RootGameObject;  // root transform must belong to a game object

    std::unique_ptr<AnimationSystem> m_AnimationSystemPtr;

    unsigned int m_VertexCount;
    unsigned int m_TriangleCount;
};

}  // namespace gdp1
