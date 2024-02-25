#include "bounds.h"

namespace gdp1 {

Bounds::Bounds(const glm::vec3& center, const glm::vec3& size) {
    m_Min = center - size * 0.5f;
    m_Max = center + size * 0.5f;
}

Bounds::Bounds()
    : m_Min(glm::vec3(0.0f, 0.0f, 0.0f))
    , m_Max(glm::vec3(0.0f, 0.0f, 0.0f)) {}

Bounds::Bounds(const Bounds& that)
    : m_Min(that.m_Min)
    , m_Max(that.m_Max) {}

void Bounds::SetCenter(const glm::vec3& center) {
    glm::vec3 ext = GetExtents();
    m_Min = center - ext;
    m_Max = center + ext;
}

void Bounds::SetMinMax(const glm::vec3& min, const glm::vec3& max) {
    this->m_Min = min;
    this->m_Max = max;
}

// Expand the bounds by increasing its size by amount along each side.
void Bounds::Expand(const glm::vec3& amount) {
    m_Min -= amount * 0.5f;
    m_Max += amount * 0.5f;
}

void Bounds::Expand(const Bounds& that) {
    glm::vec3 min = that.GetMin();
    if (min.x < this->m_Min.x) {
        this->m_Min.x = min.x;
    }
    if (min.y < this->m_Min.y) {
        this->m_Min.y = min.y;
    }
    if (min.z < this->m_Min.z) {
        this->m_Min.z = min.z;
    }

    glm::vec3 max = that.GetMax();
    if (max.x > this->m_Max.x) {
        this->m_Max.x = max.x;
    }
    if (max.y > this->m_Max.y) {
        this->m_Max.y = max.y;
    }
    if (max.z > this->m_Max.z) {
        this->m_Max.z = max.z;
    }
}

bool Bounds::Contains(const glm::vec3& point) const {
    if ((point.x > m_Min.x && point.x < m_Max.x) && (point.y > m_Min.y && point.y < m_Max.y) &&
        (point.z > m_Min.z && point.z < m_Max.z)) {
        return true;
    }
    return false;
}

bool Bounds::Intersects(const Bounds& that) const {
    glm::vec3 min = that.GetMin();
    glm::vec3 max = that.GetMax();

    if (this->m_Max.x < min.x || this->m_Max.y < min.y || this->m_Max.z < min.z) {
        return false;
    }
    if (max.x < this->m_Min.x || max.y < this->m_Min.y || max.z < this->m_Min.z) {
        return false;
    }
    return true;
}

glm::vec3 Bounds::GetCenter() const { return (m_Max + m_Min) * 0.5f; }

glm::vec3 Bounds::GetExtents() const { return (m_Max - m_Min) * 0.5f; }

glm::vec3 Bounds::GetMin() const { return m_Min; }

glm::vec3 Bounds::GetMax() const { return m_Max; }

glm::vec3 Bounds::GetSize() const { return m_Max - m_Min; }

}  // namespace gdp1
