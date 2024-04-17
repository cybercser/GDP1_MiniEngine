#pragma once

#include "common.h"

// Define the maximum number of point lights and spot lights
#define MAX_POINT_LIGHTS 2
#define MAX_SPOT_LIGHTS 1

namespace gdp1 {

class Shader;

struct DirectionalLight {
    std::string name;

    glm::vec3 direction;

    glm::vec4 color;
    float intensity;
};

struct PointLight {
    std::string name;

    glm::vec3 position;

    glm::vec4 color;
    float intensity;

    float constant;   // attenuation constant term
    float linear;     // attenuation linear term
    float quadratic;  // attenuation quadratic term
};

struct SpotLight {
    std::string name;

    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutoff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;   // attenuation constant term
    float linear;     // attenuation linear term
    float quadratic;  // attenuation quadratic term

    std::string cookie;
    unsigned int cookieTexId;

    unsigned int InitCookie();
    void EnableCookie(std::shared_ptr<Shader> shader, const glm::vec3& projUp);
    void DisableCookie(std::shared_ptr<Shader> shader);
};

struct uDirectionalLight {
    glm::vec3 dir;
    glm::vec4 color;
    float intensity;
};

struct uPointLight {
    glm::vec3 pos;
    glm::vec4 color;

    float intensity;
    float c;   // attenuation constant term
    float l;     // attenuation linear term
    float q;  // attenuation quadratic term
};

struct uSpotLight {
    glm::vec3 pos;
    glm::vec3 dir;

    float cutoff;
    float outerCutoff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float c;   // attenuation constant term
    float l;     // attenuation linear term
    float q;  // attenuation quadratic term
};

struct Lights {
    uDirectionalLight dirLight;
    uPointLight pointLights[MAX_POINT_LIGHTS];
    uSpotLight spotLights[MAX_SPOT_LIGHTS];
};

struct LightSettings {
    DirectionalLight dirLight;

    int numPointLights = 0;
    int numSpotLights = 0;

    bool useLights = true;
    bool useDirLight = true;
    bool usePointLights = true;
    bool useSpotLights = true;
};

}  // namespace gdp1
