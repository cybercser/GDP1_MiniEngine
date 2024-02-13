#pragma once
// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

namespace gdp1 {

class Timestep {
public:
    Timestep(float time = 0.0f)
        : m_Time(time) {}

    operator float() const { return m_Time; }

    float GetSeconds() const { return m_Time; }
    float GetMilliseconds() const { return m_Time * 1000.0f; }

private:
    float m_Time;
};

}  // namespace gdp1