#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gdp1 {

enum class CameraMovement {
    kFORWARD,
    kBACKWARD,
    kLEFT,
    kRIGHT,
};

class Camera {
public:
    Camera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, float fov, float aspect, float nearZ,
           float farZ);

    void Set(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, float fov, float aspect, float nearZ,
             float farZ);

    void SetViewParams(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, float fov);
    void SetProjectionParams(float aspect, float nearZ, float farZ);

    void SetEyeCenter(const glm::vec3& eye, const glm::vec3& center);
    const glm::vec3& GetEye() const;

    void SetCenterUp(const glm::vec3& center, const glm::vec3& up);
    const glm::vec3& GetCenter() const;

    const glm::vec3& GetUp() const;

    void SetAspect(float aspect);

    void SetFov(float fov);

    const glm::mat4& GetViewMatrix();
    const glm::mat4& GetProjectionMatrix();

private:
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

private:
    glm::vec3 eye_;     // eye position in world space
    glm::vec3 center_;  // look at position in world space
    glm::vec3 up_;      // up vector in world space
    float fov_;         // Y field of view angle, in degrees
    float aspect_;      // aspect ratio
    float near_z_;      // near Z plane
    float far_z_;       // far Z plane

    glm::mat4 projection_matrix_;
    glm::mat4 view_matrix_;
};

}  // namespace gdp1
