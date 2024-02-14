#pragma once
// Adapted from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL) and
// [OpenGL 4 Shading Language
// Cookbook](https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition)

#include "common.h"
#include <glad/glad.h>

namespace gdp1 {
namespace utils {

enum class DebugLogLevel {
    None = 0,
    HighAssert = 1,
    High = 2,
    Medium = 3,
    Low = 4,
    Notification = 5,
};

int CheckForGLError(const char *, int);
void DumpGLInfo(bool dumpExtensions = false);
void EnableGLDebugging();
void SetGLDebugLogLevel(DebugLogLevel level);
void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg,
                     const void *param);
}  // namespace utils
}  // namespace gdp1
