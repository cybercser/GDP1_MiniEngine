// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)
#include "windows_input.h"

#include "Core/application.h"
#include <GLFW/glfw3.h>

namespace gdp1 {

Input* Input::s_Instance = new WindowsInput();

bool WindowsInput::IsKeyPressedImpl(int keycode) {
    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    int state = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool WindowsInput::IsMouseButtonPressedImpl(int button) {
    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    int state = glfwGetMouseButton(window, button);
    return state == GLFW_PRESS;
}

std::pair<float, float> WindowsInput::GetMousePositionImpl() {
    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return {(float)xpos, (float)ypos};
}

float WindowsInput::GetMouseXImpl() {
    float x = GetMousePositionImpl().first;
    return x;
}

float WindowsInput::GetMouseYImpl() {
    float y = GetMousePositionImpl().second;
    return y;
}

}  // namespace gdp1