#pragma once
// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "Core/layer.h"

#include "Events/application_event.h"
#include "Events/key_event.h"
#include "Events/mouse_event.h"

namespace gdp1 {

class ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() = default;
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Timestep ts) override;

    void Begin();
    void End();

    virtual void OnEvent(Event& event) override;
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

private:
    float m_Time = 0.0f;
};

}  // namespace gdp1
