#pragma once
// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "Core/window.h"

#include <GLFW/glfw3.h>

namespace gdp1 {

class WindowsWindow : public Window {
public:
    WindowsWindow(const WindowProps& props);
    virtual ~WindowsWindow();

    void OnUpdate() override;

    inline uint32_t GetWidth() const override { return m_Data.width; }
    inline uint32_t GetHeight() const override { return m_Data.height; }

    // Window attributes
    inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override;

    void SetCaptureMouse(bool enabled) override;
    bool IsMouseCaptured() const override;

    void ToggleFullscreen() override;

    inline virtual void* GetNativeWindow() const { return m_Window; }

private:
    virtual void Init(const WindowProps& props);
    virtual void Shutdown();

private:
    GLFWwindow* m_Window;
    bool isFullScreen = false;

    struct WindowData {
        std::string title;
        uint32_t width, height;
        bool vSync;
        bool captureMouse;

        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

}  // namespace gdp1
