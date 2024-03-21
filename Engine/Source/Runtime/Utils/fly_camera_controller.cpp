#include "fly_camera_controller.h"

#include <functional>

#include "camera.h"
#include "Core/core.h"

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

namespace gdp1 {

FlyCameraController::FlyCameraController(CameraDesc cameraDesc, float aspect, float translationSpeed,
                                         float rotationSpeed)
    : m_Position(cameraDesc.position)
    , m_Up(cameraDesc.up)
    , m_Yaw(cameraDesc.yaw)
    , m_Pitch(cameraDesc.pitch)
    , m_Fov(cameraDesc.fov)
    , m_TranslationSpeed(translationSpeed)
    , m_RotationSpeed(rotationSpeed) {
    m_MouseSensitivity = 0.1f;

    UpdateCameraVectors();

    glm::vec3 center = m_Position + m_Forward;
    m_Camera = std::make_shared<Camera>(cameraDesc.position, center, cameraDesc.up, cameraDesc.fov, aspect, cameraDesc.nearZ,
                                        cameraDesc.farZ);
}

FlyCameraController::FlyCameraController(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov,
                                         float aspect, float nearZ, float farZ, float translationSpeed,
                                         float rotationSpeed)
    : m_Position(eye)
    , m_Up(up)
    , m_Yaw(yaw)
    , m_Pitch(pitch)
    , m_Fov(fov)
    , m_TranslationSpeed(translationSpeed)
    , m_RotationSpeed(rotationSpeed) {
    m_MouseSensitivity = 0.1f;

    UpdateCameraVectors();

    glm::vec3 center = m_Position + m_Forward;
    m_Camera = std::make_shared<Camera>(eye, center, up, fov, aspect, nearZ, farZ);
}

FlyCameraController::~FlyCameraController() {}

void FlyCameraController::SetProperty(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov,
                                      float aspect, float nearZ, float farZ) {
    assert(m_Camera != nullptr);

    m_Position = eye;
    m_Up = up;
    m_Yaw = yaw;
    m_Pitch = pitch;
    m_Fov = fov;

    UpdateCameraVectors();

    glm::vec3 center = m_Position + m_Forward;
    m_Camera->Set(eye, center, up, fov, aspect, nearZ, farZ);
}

void FlyCameraController::SetAspect(float aspect) { m_Camera->SetAspect(aspect); }

const glm::mat4& FlyCameraController::GetViewMatrix() const { return m_Camera->GetViewMatrix(); }

const glm::mat4& FlyCameraController::GetProjectionMatrix() const { return m_Camera->GetProjectionMatrix(); }

void FlyCameraController::OnUpdate(Timestep ts) {
    if (Input::IsKeyPressed(HZ_KEY_W)) {
        ProcessKeyboard(CameraMovement::kFORWARD, ts);
    }
    if (Input::IsKeyPressed(HZ_KEY_S)) {
        ProcessKeyboard(CameraMovement::kBACKWARD, ts);
    }
    if (Input::IsKeyPressed(HZ_KEY_A)) {
        ProcessKeyboard(CameraMovement::kLEFT, ts);
    }
    if (Input::IsKeyPressed(HZ_KEY_D)) {
        ProcessKeyboard(CameraMovement::kRIGHT, ts);
    }
}

void FlyCameraController::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>(GLCORE_BIND_EVENT_FN(FlyCameraController::OnMouseScrolled));
    dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(FlyCameraController::OnWindowResized));
    dispatcher.Dispatch<MouseMovedEvent>(GLCORE_BIND_EVENT_FN(FlyCameraController::OnMouseMoved));
}

void FlyCameraController::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float speed = m_TranslationSpeed * deltaTime;
    glm::vec3 translation_{0.0f, 0.0f, 0.0f};
    switch (direction) {
        case CameraMovement::kFORWARD:
            translation_ = m_Forward * speed;
            break;
        case CameraMovement::kBACKWARD:
            translation_ = -m_Forward * speed;
            break;
        case CameraMovement::kLEFT:
            translation_ = -m_Right * speed;
            break;
        case CameraMovement::kRIGHT:
            translation_ = m_Right * speed;
            break;
    }
    m_Position += translation_;

    glm::vec3 center = m_Position + m_Forward;
    m_Camera->SetEyeCenter(m_Position, center);
}

void FlyCameraController::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_MouseSensitivity * m_RotationSpeed;
    yoffset *= m_MouseSensitivity * m_RotationSpeed;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (m_Pitch > 89.0f) m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }

    // update Forward, Right and Up Vectors using the updated Euler angles
    UpdateCameraVectors();

    glm::vec3 center = m_Position + m_Forward;
    m_Camera->SetCenterUp(center, m_Up);
}

void FlyCameraController::ProcessMouseScroll(float yoffset) {
    m_Fov -= (float)yoffset;
    if (m_Fov < 1.0f) m_Fov = 1.0f;
    if (m_Fov > 60.0f) m_Fov = 60.0f;

    m_Camera->SetFov(m_Fov);
}

void FlyCameraController::UpdateCameraVectors() {
    // calculate the new forward vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Forward = glm::normalize(front);

    // also re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Forward, kWORLD_UP));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}

bool FlyCameraController::OnMouseScrolled(MouseScrolledEvent& e) {
    ProcessMouseScroll(e.GetYOffset());
    return false;  // event is not handled
}

bool FlyCameraController::OnWindowResized(WindowResizeEvent& e) {
    m_Camera->SetAspect((float)e.GetWidth() / (float)e.GetHeight());
    return false;  // event is not handled
}

bool FlyCameraController::OnMouseMoved(MouseMovedEvent& e) {
    static bool firstMouse = true;

    if (firstMouse) {
        m_LastX = e.GetX();
        m_LastY = e.GetY();
        firstMouse = false;
    }

    float xoffset = e.GetX() - m_LastX;
    float yoffset = m_LastY - e.GetY();  // reversed since y-coordinates go from bottom to top

    m_LastX = e.GetX();
    m_LastY = e.GetY();

    ProcessMouseMovement(xoffset, yoffset, true);
    return false;  // event is not handled
}

}  // namespace gdp1
