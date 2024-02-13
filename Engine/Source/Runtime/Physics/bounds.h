#pragma once

#include "common.h"

namespace gdp1 {
// comments copied from unity docs: https://docs.unity3d.com/ScriptReference/Bounds.html
// Represents an axis aligned bounding box.
// An axis - aligned bounding box, or AABB for short, is a box aligned with coordinate axes and fully enclosing some
// object. Because the box is never rotated with respect to the axes, it can be defined by just its center and extents,
// or alternatively by min and max points.
class Bounds {
public:
    Bounds();
    Bounds(const glm::vec3& center, const glm::vec3& size);
    Bounds(const Bounds& rhs);

    // setters
    void SetCenter(const glm::vec3& center);
    void SetMinMax(const glm::vec3& min, const glm::vec3& max);
    void Expand(const glm::vec3& amount);
    void Expand(const Bounds& bounds);

    // checks
    bool Contains(const glm::vec3& point) const;
    bool Intersects(const Bounds& bounds) const;

    // getters
    glm::vec3 GetCenter() const;
    glm::vec3 GetExtents() const;

    glm::vec3 GetMin() const;
    glm::vec3 GetMax() const;

    glm::vec3 GetSize() const;

private:
    glm::vec3 min_;
    glm::vec3 max_;
};

}  // namespace gdp1
