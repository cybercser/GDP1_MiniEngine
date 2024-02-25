#pragma once

#include "common.h"
#include "Render/light.h"

namespace gdp1 {

// Camera description
struct CameraDesc {
    std::string name;
    bool main;
    glm::vec3 position;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;
    float nearZ;
    float farZ;
};

// Collider description
struct ColliderDesc {
    std::string name;
    std::string objectName;
    std::string shape;
    bool isTrigger;
};

// Rigidbody description
struct RigidbodyDesc {
    std::string objectName;
    bool active;
    std::string colliderName;
    float invMass;
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 velocity;
};

// Transform description
struct TransformDesc {
    glm::vec3 localPosition;
    glm::vec3 localEulerAngles;
    glm::vec3 localScale;
};

// GameObject description
struct GameObjectDesc {
    std::string name;
    std::string modelName;
    bool visible;
    TransformDesc transform;
    std::vector<std::string> children;
    std::string parentName;
};

// Model description
struct ModelDesc {
    std::string name;
    std::string filepath;
    std::string shader;
};

// AnimationReference description
struct AnimationRefDesc {
    std::string name;
    std::string path;
};

// AudioSource description
struct AudioSourceDesc {
    std::string name;
    std::string filepath;
    float volume;
    float pitch;
    float pan;
    bool streamed;
    bool loop;
    bool spatial;
    bool playOnAwake;
    glm::vec3 position;
    float minDistance;
    float maxDistance;
};

// Skybox description
struct SkyboxDesc {
    std::string name;
    std::vector<std::string> faces;
    float size;
};

// game level description
struct LevelDesc {
    std::string name;
    std::string comment;
    std::string version;
    std::vector<CameraDesc> cameraDescs;
    std::vector<DirectionalLight> directionalLights;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    std::vector<ModelDesc> modelDescs;
    std::vector<GameObjectDesc> gameObjectDescs;
    std::vector<RigidbodyDesc> rigidbodyDescs;
    std::vector<AudioSourceDesc> audioSourceDescs;
    AnimationRefDesc animationRefDesc;
    SkyboxDesc skyboxDesc;
};

}  // namespace gdp1
