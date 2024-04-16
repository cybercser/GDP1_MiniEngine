#pragma once
// Adapted from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "window.h"
#include "layer_stack.h"
#include "Events/event.h"
#include "Events/application_event.h"

#include "timestep.h"

#include "ImGui/imgui_layer.h"

#include <lua.hpp>

#include "cs_runner.h"

namespace gdp1 {

// forward declaration

class Application : public CSRunner {
public:
    Application(const std::string& name = "MiniEngine App", unsigned int width = 0, unsigned int height = 0, bool startFullScreen = false);
    virtual ~Application() = default;

    void Run();

    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    inline Window& GetWindow() { return *m_Window; }

    inline static Application& Get() { return *s_Instance; }

    lua_State* lua_state;

    static int drawCalls;

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
    float m_AccumulatedTime = 0.0f;
    float m_FPS = 0.0f;
    float m_MS = 0.0f;

    unsigned int m_Frames = 0;

    static Application* s_Instance;
};

}  // namespace gdp1
