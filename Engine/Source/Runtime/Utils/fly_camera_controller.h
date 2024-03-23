#pragma once

#include "common.h"

#include "Core/timestep.h"
#include "Events/event.h"
#include "Events/key_event.h"
#include "Events/mouse_event.h"
#include "Events/application_event.h"
#include "Resource/level_object_description.h"

namespace gdp1 {

// Forward declaration
class Camera;

enum class CameraMovement {
    kFORWARD,
    kBACKWARD,
    kLEFT,
    kRIGHT,
};

class FlyCameraController {
public:
    FlyCameraController(CameraDesc cameraDesc, float aspect, float translationSpeed, float rotationSpeed);

    FlyCameraController(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov, float aspect,
                        float nearZ, float farZ, float translationSpeed, float rotationSpeed);

    ~FlyCameraController();

    void SetProperty(const glm::vec3& eye, const glm::vec3& up, float fov, float yaw, float pitch, float aspect,
                     float nearZ, float farZ);

    void SetAspect(float aspect);

    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    const glm::vec3& GetPosition() const { return m_Position; }

    const glm::vec3& GetUp() const { return m_Up; }

    float GetYaw() const { return m_Yaw; }

    float GetPitch() const { return m_Pitch; }

    float GetFov() const { return m_Fov; }

    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;

    void OnUpdate(Timestep ts);
    void OnEvent(Event& e);

private:
    void UpdateCameraVectors();

    bool OnMouseScrolled(MouseScrolledEvent& e);
    bool OnWindowResized(WindowResizeEvent& e);
    bool OnMouseMoved(MouseMovedEvent& e);

    // WASD for forward, strafe left, backward, strafe right
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Yaw and pitch. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);

    // Zoom-in/out, only cares about vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

private:
    std::shared_ptr<Camera> m_Camera;

    glm::vec3 m_Position;
    glm::vec3 m_Forward;
    glm::vec3 m_Up;
    glm::vec3 m_Right;

    float m_Yaw;
    float m_Pitch;

    float m_Fov;

    const glm::vec3 kWORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_TranslationSpeed;
    float m_RotationSpeed;
    float m_MouseSensitivity;

    float m_LastX;
    float m_LastY;

    bool firstMouse = true;
};

}  // namespace gdp1
