#pragma once

// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "core.h"
#include "events/event.h"

#include <functional>

namespace gdp1 {

struct WindowProps {
    std::string title;
    uint32_t width;
    uint32_t height;

    WindowProps(const std::string& title = "GDP1 MiniEngine", uint32_t width = 1280, uint32_t height = 720)
        : title(title)
        , width(width)
        , height(height) {}
};

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    // Window attributes
    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;
    virtual void SetCaptureMouse(bool enabled) = 0;
    virtual bool IsMouseCaptured() const = 0;

    virtual void* GetNativeWindow() const = 0;

    static Window* Create(const WindowProps& props = WindowProps());
};
}  // namespace gdp1
