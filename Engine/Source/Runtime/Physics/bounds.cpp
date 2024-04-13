#include "bounds.h"

namespace gdp1 {

Bounds::Bounds(const glm::vec3& center, const glm::vec3& size) {
    min_ = center - size * 0.5f;
    max_ = center + size * 0.5f;
}

Bounds::Bounds()
    : min_(glm::vec3(0.0f, 0.0f, 0.0f))
    , max_(glm::vec3(0.0f, 0.0f, 0.0f)) {}

Bounds::Bounds(const Bounds& that)
    : min_(that.min_)
    , max_(that.max_) {}

void Bounds::SetCenter(const glm::vec3& center) {
    glm::vec3 ext = GetExtents();
    min_ = center - ext;
    max_ = center + ext;
}

void Bounds::SetMinMax(const glm::vec3& min, const glm::vec3& max) {
    this->min_ = min;
    this->max_ = max;
}

// Expand the bounds by increasing its size by amount along each side.
void Bounds::Expand(const glm::vec3& amount) {
    min_ -= amount * 0.5f;
    max_ += amount * 0.5f;
}

void Bounds::Expand(const Bounds& that) {
    glm::vec3 min = that.GetMin();
    if (min.x < this->min_.x) {
        this->min_.x = min.x;
    }
    if (min.y < this->min_.y) {
        this->min_.y = min.y;
    }
    if (min.z < this->min_.z) {
        this->min_.z = min.z;
    }

    glm::vec3 max = that.GetMax();
    if (max.x > this->max_.x) {
        this->max_.x = max.x;
    }
    if (max.y > this->max_.y) {
        this->max_.y = max.y;
    }
    if (max.z > this->max_.z) {
        this->max_.z = max.z;
    }
}

bool Bounds::Contains(const glm::vec3& point) const {
    if ((point.x > min_.x && point.x < max_.x) && (point.y > min_.y && point.y < max_.y) &&
        (point.z > min_.z && point.z < max_.z)) {
        return true;
    }
    return false;
}

bool Bounds::Intersects(const Bounds& that) const {
    glm::vec3 min = that.GetMin();
    glm::vec3 max = that.GetMax();

    if (this->max_.x < min.x || this->max_.y < min.y || this->max_.z < min.z) {
        return false;
    }
    if (max.x < this->min_.x || max.y < this->min_.y || max.z < this->min_.z) {
        return false;
    }
    return true;
}

glm::vec3 Bounds::GetCenter() const { return (max_ + min_) * 0.5f; }

glm::vec3 Bounds::GetExtents() const { return (max_ - min_) * 0.5f; }

glm::vec3 Bounds::GetMin() const { return min_; }

glm::vec3 Bounds::GetMax() const { return max_; }

glm::vec3 Bounds::GetSize() const { return max_ - min_; }

void Bounds::TransformBounds(glm::mat4& worldMatrix) {
    // Transform the min and max points of the bounding box by the world matrix
    glm::vec3 transformedMin = glm::vec3(worldMatrix * glm::vec4(min_, 1.0f));
    glm::vec3 transformedMax = glm::vec3(worldMatrix * glm::vec4(max_, 1.0f));

    SetMinMax(glm::min(transformedMin, transformedMax), glm::max(transformedMin, transformedMax));
}

}  // namespace gdp1
