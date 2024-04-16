#include "application.h"

#include "Core/logger.h"
#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

using namespace glm;

namespace gdp1 {

int Application::drawCalls = 0;

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name, unsigned int width, unsigned int height, bool startFullScreen) {
    if (!s_Instance) {
        Logger::Init();
    } 

    GLCORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(name, width, height)));
    //if (startFullScreen) m_Window->ToggleFullscreen();

    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);
}

void Application::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
    dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
    dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(OnMouseButtonPressed));

    // LOG_TRACE("{0}", e);

    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
        Layer* layer = *--it;

        // if the mouse is not captured and the layer is not an overlay, skip it
        if (!m_Window->IsMouseCaptured() && !m_LayerStack.IsOverlay(layer)) {
            continue;
        }
        layer->OnEvent(e);
        if (e.handled) {
            break;
        }
    }
}

void Application::PushLayer(Layer* layer) { m_LayerStack.PushLayer(layer); }

void Application::PushOverlay(Layer* overlay) { m_LayerStack.PushOverlay(overlay); }

void Application::Run() {
    while (m_Running) {
        drawCalls = 0;

        float time = (float)glfwGetTime();
        Timestep timestep = time - m_LastFrameTime;
        m_LastFrameTime = time;

         // Calculate FPS and MS
        m_Frames++;
        m_AccumulatedTime += timestep;
        if (m_AccumulatedTime >= 1.0f) {
            m_FPS = m_Frames;
            m_MS = 1000.0f / m_FPS;
            m_Frames = 0;
            m_AccumulatedTime = 0.0f;
        }

        m_Window->OnUpdate();

        for (Layer* layer : m_LayerStack) {
            layer->OnUpdate(timestep);
        }

        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack) {
            layer->OnImGuiRender();
        }

        // Render FPS and MS in ImGui dialog
        ImGui::Begin("Performance Metrics");
        ImGui::Text("FPS: %.3f", m_FPS);
        ImGui::Text("MS per Frame: %.3f", m_MS);
        ImGui::Text("Timestep: %.4f", timestep.GetSeconds());
        ImGui::Text("Draw Calls: %d", drawCalls);
        ImGui::End();

        m_ImGuiLayer->End();
    }
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    return true;
}

bool Application::OnKeyPressed(KeyPressedEvent& e) {
    if (e.GetKeyCode() == HZ_KEY_ESCAPE) {
        m_Window->SetCaptureMouse(false);
    }
    return false;
}

bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT)) {
        m_Window->SetCaptureMouse(true);
    }
    return false;
}

}  // namespace gdp1
