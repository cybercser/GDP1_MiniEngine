#include "fly_camera_controller.h"

#include "camera.h"

namespace gdp1 {

FlyCameraController::FlyCameraController(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov,
                                         float aspect, float nearZ, float farZ, float translationSpeed)
    : position_(eye)
    , up_(up)
    , yaw_(yaw)
    , pitch_(pitch)
    , fov_(fov)
    , translation_speed_(translationSpeed) {
    mouse_sensitivity_ = 0.1f;

    UpdateCameraVectors();

    glm::vec3 center = position_ + forward_;
    camera_ptr_ = std::make_shared<Camera>(eye, center, up, fov, aspect, nearZ, farZ);
}

FlyCameraController::~FlyCameraController() {
}

void FlyCameraController::SetProperty(const glm::vec3& eye, const glm::vec3& up, float yaw, float pitch, float fov,
                                      float aspect, float nearZ, float farZ) {
    assert(camera_ptr_ != nullptr);

    position_ = eye;
    up_ = up;
    yaw_ = yaw;
    pitch_ = pitch;
    fov_ = fov;

    UpdateCameraVectors();

    glm::vec3 center = position_ + forward_;
    camera_ptr_->Set(eye, center, up, fov, aspect, nearZ, farZ);
}

void FlyCameraController::SetAspect(float aspect) {
    camera_ptr_->SetAspect(aspect);
}

const glm::mat4& FlyCameraController::GetViewMatrix() const {
    return camera_ptr_->GetViewMatrix();
}

const glm::mat4& FlyCameraController::GetProjectionMatrix() const {
    return camera_ptr_->GetProjectionMatrix();
}

void FlyCameraController::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float speed = translation_speed_ * deltaTime;
    glm::vec3 translation_{0.0f, 0.0f, 0.0f};
    switch (direction) {
        case FORWARD:
            translation_ = forward_ * speed;
            break;
        case BACKWARD:
            translation_ = -forward_ * speed;
            break;
        case LEFT:
            translation_ = -right_ * speed;
            break;
        case RIGHT:
            translation_ = right_ * speed;
            break;
    }
    position_ += translation_;

    glm::vec3 center = position_ + forward_;
    camera_ptr_->SetEyeCenter(position_, center);
}

void FlyCameraController::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouse_sensitivity_;
    yoffset *= mouse_sensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (pitch_ > 89.0f) pitch_ = 89.0f;
        if (pitch_ < -89.0f) pitch_ = -89.0f;
    }

    // update Forward, Right and Up Vectors using the updated Euler angles
    UpdateCameraVectors();

    glm::vec3 center = position_ + forward_;
    camera_ptr_->SetCenterUp(center, up_);
}

void FlyCameraController::ProcessMouseScroll(float yoffset) {
    fov_ -= (float)yoffset;
    if (fov_ < 1.0f) fov_ = 1.0f;
    if (fov_ > 60.0f) fov_ = 60.0f;

    camera_ptr_->SetFov(fov_);
}

void FlyCameraController::UpdateCameraVectors() {
    // calculate the new forward vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    forward_ = glm::normalize(front);

    // also re-calculate the Right and Up vector
    right_ = glm::normalize(glm::cross(forward_, kWORLD_UP));
    up_ = glm::normalize(glm::cross(right_, forward_));
}

}  // namespace gdp1
