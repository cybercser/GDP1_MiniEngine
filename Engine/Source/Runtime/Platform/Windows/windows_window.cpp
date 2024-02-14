// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)
#include "windows_window.h"

#include "Events/application_event.h"
#include "Events/key_event.h"
#include "Events/mouse_event.h"
#include "Utils/glutils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace gdp1 {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* description) {
    LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}

Window* Window::Create(const WindowProps& props) { return new WindowsWindow(props); }

WindowsWindow::WindowsWindow(const WindowProps& props) { Init(props); }

WindowsWindow::~WindowsWindow() { Shutdown(); }

void WindowsWindow::Init(const WindowProps& props) {
    m_Data.title = props.title;
    m_Data.width = props.width;
    m_Data.height = props.height;

    LOG_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

    if (!s_GLFWInitialized) {
        int success = glfwInit();
        GLCORE_ASSERT(success, "Could not initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);

        // Select OpenGL 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);  // disable window resize
        // if (gl_debug_) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        // if (multi_samples_ > 0) {
        //     glfwWindowHint(GLFW_SAMPLES, multi_samples_);
        // }

        s_GLFWInitialized = true;
    }

    m_Window = glfwCreateWindow((int)props.width, (int)props.height, m_Data.title.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(m_Window);
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    GLCORE_ASSERT(status, "Failed to initialize Glad!");

    utils::DumpGLInfo();

    glfwSetWindowUserPointer(m_Window, &m_Data);
    SetVSync(true);

    // Set GLFW callbacks
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.width = width;
        data.height = height;

        WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        data.EventCallback(event);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key, 0);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(key, 1);
                data.EventCallback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        KeyTypedEvent event(keycode);
        data.EventCallback(event);
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(button);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(button);
                data.EventCallback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.EventCallback(event);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseMovedEvent event((float)xPos, (float)yPos);
        data.EventCallback(event);
    });
}

void WindowsWindow::Shutdown() { glfwDestroyWindow(m_Window); }

void WindowsWindow::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

void WindowsWindow::SetVSync(bool enabled) {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    m_Data.vSync = enabled;
}

bool WindowsWindow::IsVSync() const { return m_Data.vSync; }

}  // namespace gdp1
