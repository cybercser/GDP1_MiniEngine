#pragma once
// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "core.h"
#include "timestep.h"
#include "Events/event.h"

#include <string>

namespace gdp1 {

class Layer {
public:
    Layer(const std::string& name = "Layer")
        : m_DebugName(name) {}

    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(Timestep ts) {}
    virtual void OnImGuiRender() {}
    virtual void OnEvent(Event& event) {}

    const std::string& GetName() const { return m_DebugName; }

protected:
    std::string m_DebugName;
};

}  // namespace gdp1
