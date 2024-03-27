#include "fps_camera_controller.h"

#include <functional>

#include "camera.h"
#include "Core/core.h"

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

namespace gdp1 {

FPSCameraController::FPSCameraController(CameraDesc cameraDesc, float aspect, float translationSpeed, float rotationSpeed)
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
    m_Camera = std::make_shared<Camera>(cameraDesc.position, center, cameraDesc.up, cameraDesc.fov, aspect,
                                        cameraDesc.nearZ, cameraDesc.farZ);
}

FPSCameraController::~FPSCameraController() {}

void FPSCameraController::SetAspect(float aspect) { m_Camera->SetAspect(aspect); }

const glm::mat4& FPSCameraController::GetViewMatrix() const { return m_Camera->GetViewMatrix(); }

const glm::mat4& FPSCameraController::GetProjectionMatrix() const { return m_Camera->GetProjectionMatrix(); }

void FPSCameraController::OnUpdate(Timestep ts) {
    
}

void FPSCameraController::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseMovedEvent>(GLCORE_BIND_EVENT_FN(FPSCameraController::OnMouseMoved));
    dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(FPSCameraController::OnWindowResized));
}

void FPSCameraController::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
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

void FPSCameraController::ProcessMouseScroll(float yoffset) {
    m_Fov -= (float)yoffset;
    if (m_Fov < 1.0f) m_Fov = 1.0f;
    if (m_Fov > 60.0f) m_Fov = 60.0f;

    m_Camera->SetFov(m_Fov);
}

void FPSCameraController::UpdateCameraVectors() {
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

bool FPSCameraController::OnMouseScrolled(MouseScrolledEvent& e) {
    ProcessMouseScroll(e.GetYOffset());
    return false;  // event is not handled
}

bool FPSCameraController::OnWindowResized(WindowResizeEvent& e) {
    m_Camera->SetAspect((float)e.GetWidth() / (float)e.GetHeight());
    return false;  // event is not handled
}

bool FPSCameraController::OnMouseMoved(MouseMovedEvent& e) {
    if (firstMouse) {
        m_LastX = e.GetX();
        m_LastY = e.GetY();
        firstMouse = false;
    }

    float xoffset = e.GetX() - m_LastX;
    float yoffset = m_LastY - e.GetY();

    m_LastX = e.GetX();
    m_LastY = e.GetY();

    ProcessMouseMovement(xoffset, yoffset, true);
    return false;  // event is not handled
}

void FPSCameraController::SetPosition(glm::vec3 pos) {
    m_Position = pos;
}

}  // namespace gdp1
