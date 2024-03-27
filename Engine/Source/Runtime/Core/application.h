#pragma once
// Adapted from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "window.h"
#include "layer_stack.h"
#include "Events/event.h"
#include "Events/application_event.h"

#include "timestep.h"

#include "ImGui/imgui_layer.h"

#include <lua.hpp>

namespace gdp1 {

// forward declaration

class Application {
public:
    Application(const std::string& name = "MiniEngine App", unsigned int width = 1920, unsigned int height = 1080, bool startFullScreen = true);
    virtual ~Application() = default;

    void Run();

    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    inline Window& GetWindow() { return *m_Window; }

    inline static Application& Get() { return *s_Instance; }

    lua_State* lua_state;

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

private:
    std::unique_ptr<Window> m_Window;
    ImGuiLayer* m_ImGuiLayer;
    bool m_Running = true;
    LayerStack m_LayerStack;
    float m_LastFrameTime = 0.0f;

    static Application* s_Instance;
};

}  // namespace gdp1
