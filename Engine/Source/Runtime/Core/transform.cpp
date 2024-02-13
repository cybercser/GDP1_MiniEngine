#include "transform.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace gdp1 {

Transform::Transform(GameObject* go, const TransformDesc& desc)
    : gameObject(go)
    , parent(nullptr)
    , root(nullptr)
    , localPosition(desc.localPosition)
    , localEulerAngles(desc.localEulerAngles)
    , localScale(desc.localScale)
    , hasChanged(true)
    , m_NeedToUpdateLocalMatrix(true)
    , m_NeedToUpdateLocalRotation(true) {
    UpdateLocalMatrix();
}

Transform::Transform(GameObject* go)
    : gameObject(go)
    , parent(nullptr)
    , root(nullptr)
    , localPosition(glm::vec3(0.0f))
    , localEulerAngles(glm::vec3(0.0f))
    , localScale(glm::vec3(1.0f))
    , localRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
    , m_LocalMatrix(glm::mat4(1.0f))
    , m_Position(glm::vec3(0.0f))
    , m_Scale(glm::vec3(1.0f))
    , m_Rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
    , m_WorldMatrix(glm::mat4(1.0f))
    , hasChanged(true)
    , m_NeedToUpdateLocalMatrix(true)
    , m_NeedToUpdateLocalRotation(true) {
}

Transform::~Transform() {
}

glm::vec3 Transform::Position() const {
    return m_Position;
}

void Transform::SetPosition(const glm::vec3& pos) {
    localPosition = glm::vec3(WorldToLocalMatrix() * glm::vec4(pos, 1.0f));
    m_NeedToUpdateLocalRotation = false;
    m_NeedToUpdateLocalMatrix = true;
    hasChanged = true;
}

glm::vec3 Transform::EulerAngles() const {
    return glm::degrees(glm::eulerAngles(m_Rotation));
}

void Transform::SetEulerAngles(const glm::vec3& anglesInDegree) {
    glm::quat rot = glm::quat(glm::radians(anglesInDegree));
    localRotation = glm::quat(WorldToLocalMatrix()) * rot;
    m_NeedToUpdateLocalRotation = true;
    m_NeedToUpdateLocalMatrix = true;
    hasChanged = true;
}

glm::vec3 Transform::Scale() const {
    return m_Scale;
}

void Transform::SetScale(const glm::vec3& scale) {
    localScale = glm::vec3(WorldToLocalMatrix() * glm::vec4(scale, 1.0f));
    m_NeedToUpdateLocalRotation = false;
    m_NeedToUpdateLocalMatrix = true;
    hasChanged = true;
}

glm::quat Transform::Rotation() const {
    return m_Rotation;
}

void Transform::SetRotation(const glm::quat& rot) {
    localRotation = glm::quat(WorldToLocalMatrix()) * rot;
    localEulerAngles = glm::degrees(glm::eulerAngles(localRotation));
    m_NeedToUpdateLocalRotation = false;
    m_NeedToUpdateLocalMatrix = true;
    hasChanged = true;
}

glm::mat4 Transform::LocalMatrix() const {
    // if (m_NeedToUpdateLocalMatrix) {
    //     UpdateLocalMatrix();
    // }
    return m_LocalMatrix;
}

glm::mat4 Transform::WorldMatrix() const {
    return m_WorldMatrix;
}

void Transform::SetWorldMatrix(const glm::mat4& mat) {
    m_LocalMatrix = glm::mat4(WorldToLocalMatrix()) * mat;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_LocalMatrix, localScale, localRotation, localPosition, skew, perspective);
    m_WorldMatrix = mat;
    m_NeedToUpdateLocalMatrix = false;
    hasChanged = true;
}

glm::mat4 Transform::LocalToWorldMatrix() const {
    if (parent == nullptr) {
        return glm::mat4(1.0f);
    }
    return parent->WorldMatrix();
}

glm::mat4 Transform::WorldToLocalMatrix() const {
    if (parent == nullptr) {
        return glm::mat4(1.0f);
    }
    return glm::inverse(parent->WorldMatrix());
}

void Transform::UpdateLocalMatrix() {
    if (!m_NeedToUpdateLocalMatrix) {
        return;
    }

    m_LocalMatrix = glm::mat4(1.0f);
    m_LocalMatrix = glm::translate(m_LocalMatrix, localPosition);
    m_LocalMatrix = glm::scale(m_LocalMatrix, localScale);

    if (m_NeedToUpdateLocalRotation) {
        glm::vec3 eulerAnglesRadian = glm::radians(localEulerAngles);
        localRotation = glm::quat(eulerAnglesRadian);
    }

    m_LocalMatrix = m_LocalMatrix * glm::mat4(localRotation);
}

}  // namespace gdp1
