#include "camera.h"

namespace gdp1 {

Camera::Camera(const glm::vec3& eye, const glm::vec3& lookat, const glm::vec3& up, float fov, float aspect,
               float near_z, float far_z) {
    Set(eye, lookat, up, fov, aspect, near_z, far_z);
}

void Camera::Set(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, float fov, float aspect,
                 float nearZ, float farZ) {
    eye_ = eye;
    center_ = center;
    up_ = up;
    fov_ = fov;
    aspect_ = aspect;
    near_z_ = nearZ;
    far_z_ = farZ;

    RecalculateViewMatrix();
    RecalculateProjectionMatrix();
}

void Camera::SetViewParams(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, float fov) {
    eye_ = eye;
    center_ = center;
    up_ = up;
    fov_ = fov;

    RecalculateViewMatrix();
}

void Camera::SetProjectionParams(float aspect, float nearZ, float farZ) {
    aspect_ = aspect;
    near_z_ = nearZ;
    far_z_ = farZ;

    RecalculateProjectionMatrix();
}

void Camera::SetEyeCenter(const glm::vec3& eye, const glm::vec3& center) {
    eye_ = eye;
    center_ = center;
    RecalculateViewMatrix();
}

const glm::vec3& Camera::GetEye() const { return eye_; }

void Camera::SetCenterUp(const glm::vec3& center, const glm::vec3& up) {
    center_ = center;
    up_ = up;
    RecalculateViewMatrix();
}

const glm::vec3& Camera::GetCenter() const { return center_; }

const glm::vec3& Camera::GetUp() const { return up_; }

void Camera::SetAspect(float aspect) {
    aspect_ = aspect;
    RecalculateProjectionMatrix();
}

void Camera::SetFov(float fov) {
    fov_ = fov;
    RecalculateProjectionMatrix();
}

const glm::mat4& Camera::GetViewMatrix() { return view_matrix_; }

const glm::mat4& Camera::GetProjectionMatrix() { return projection_matrix_; }

const glm::mat4& Camera::GetViewProjectionMatrix() { return projection_matrix_ * view_matrix_; }

void Camera::RecalculateViewMatrix() { view_matrix_ = glm::lookAt(eye_, center_, up_); }

void Camera::RecalculateProjectionMatrix() {
    projection_matrix_ = glm::perspective(glm::radians(fov_), aspect_, near_z_, far_z_);
}

}  // namespace gdp1
