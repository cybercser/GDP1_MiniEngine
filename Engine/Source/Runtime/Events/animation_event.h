#pragma once

#include "event.h"

#include "common.h"

namespace gdp1 {
class AnimationEvent : public Event {
public:
    inline float time() const { return m_Time; }

    EVENT_CLASS_CATEGORY(EventCategoryAnimation)

protected:
    AnimationEvent(float time)
        : m_Time(time) {}

    float m_Time;
};

class AnimationChangeColorEvent : public AnimationEvent {
public:
    AnimationChangeColorEvent(float time, const glm::vec3& color)
        : AnimationEvent(time)
        , m_Color(color) {}

    inline const glm::vec3& color() const { return m_Color; }

    std::string ToString() const override {
        return fmt::format("KeyframeEvent: time {}, color ({}, {}, {})", m_Time, m_Color.r, m_Color.g, m_Color.b);
    }

    EVENT_CLASS_TYPE(AnimationChangeColor)
private:
    glm::vec3 m_Color;
};

}  // namespace gdp1