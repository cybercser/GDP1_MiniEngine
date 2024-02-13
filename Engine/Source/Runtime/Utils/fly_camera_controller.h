#pragma once

#include "common.h"

namespace gdp1 {

// Forward declaration
class Camera;

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

class FlyCameraController {
public:
    FlyCameraController(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov, float aspect,
                        float nearZ, float farZ, float translationSpeed);

    ~FlyCameraController();

    void SetProperty(const glm::vec3& eye, const glm::vec3& up, float fov, float yaw, float pitch, float aspect,
                     float nearZ, float farZ);

    void SetAspect(float aspect);

    // std::shared_ptr<Camera> GetCamera() const { return camera_ptr_; }

    const glm::vec3& GetPosition() const {
        return position_;
    }

    const glm::vec3& GetUp() const {
        return up_;
    }

    float GetYaw() const {
        return yaw_;
    }

    float GetPitch() const {
        return pitch_;
    }

    float GetFov() const {
        return fov_;
    }

    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;

    // WASD for forward, strafe left, backward, strafe right
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Yaw and pitch. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);

    // Zoom-in/out, only cares about vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

private:
    void UpdateCameraVectors();

private:
    std::shared_ptr<Camera> camera_ptr_;

    glm::vec3 position_;
    glm::vec3 forward_;
    glm::vec3 up_;
    glm::vec3 right_;

    float yaw_;
    float pitch_;

    float fov_;

    // Default camera values
    const float kSPEED = 300.0f;
    const float kSENSITIVITY = 0.1f;
    const float kFOV = 45.0f;
    const glm::vec3 kWORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    float translation_speed_;
    float rotation_speed_;
    float mouse_sensitivity_;
};

}  // namespace gdp1
