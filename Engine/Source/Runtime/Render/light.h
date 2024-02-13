#pragma once

#include "common.h"

namespace gdp1 {

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

class Shader;
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

}  // namespace gdp1
