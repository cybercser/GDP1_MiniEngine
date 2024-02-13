#pragma once

#include "common.h"
#include <glad/glad.h>

namespace gdp1 {
namespace GLUtils {
int checkForOpenGLError(const char *, int);

void dumpGLInfo(bool dumpExtensions = false);

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg,
                            const void *param);
}  // namespace GLUtils
}  // namespace gdp1
