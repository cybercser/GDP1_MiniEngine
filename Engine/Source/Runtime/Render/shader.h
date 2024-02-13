#pragma once
// Adapted from https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition

#ifdef WIN32
#pragma warning(disable : 4290)
#endif

#include "common.h"

#include <glad/glad.h>

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <stdexcept>

namespace gdp1 {

class GLSLProgramException : public std::runtime_error {
public:
    GLSLProgramException(const std::string& msg)
        : std::runtime_error(msg) {}
};

namespace GLSLShader {
enum GLSLShaderType {
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    GEOMETRY = GL_GEOMETRY_SHADER,
    TESS_CONTROL = GL_TESS_CONTROL_SHADER,
    TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
    COMPUTE = GL_COMPUTE_SHADER
};
};

class Shader {
private:
    GLuint handle;
    bool linked;
    std::map<std::string, int> uniformLocations;

    inline GLint getUniformLocation(const char* name);
    void detachAndDeleteShaderObjects();
    bool fileExists(const std::string& fileName);
    std::string getExtension(const char* fileName);

public:
    Shader();
    ~Shader();

    // Make it non-copyable.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void compileShader(const char* fileName);
    void compileShader(const char* fileName, GLSLShader::GLSLShaderType type);
    void compileShader(const std::string& source, GLSLShader::GLSLShaderType type, const char* fileName = NULL);

    void link();
    void validate();
    void use();

    int getHandle();
    bool isLinked();

    void bindAttribLocation(GLuint location, const char* name);
    void bindFragDataLocation(GLuint location, const char* name);

    // std::string version of the following methods
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, const glm::vec2& v);
    void setUniform(const std::string& name, const glm::vec3& v);
    void setUniform(const std::string& name, const glm::vec4& v);
    void setUniform(const std::string& name, const glm::mat4& m);
    void setUniform(const std::string& name, const glm::mat3& m);
    void setUniform(const std::string& name, float val);
    void setUniform(const std::string& name, int val);
    void setUniform(const std::string& name, bool val);
    void setUniform(const std::string& name, GLuint val);

    void setUniform(const char* name, float x, float y, float z);
    void setUniform(const char* name, const glm::vec2& v);
    void setUniform(const char* name, const glm::vec3& v);
    void setUniform(const char* name, const glm::vec4& v);
    void setUniform(const char* name, const glm::mat4& m);
    void setUniform(const char* name, const glm::mat3& m);
    void setUniform(const char* name, float val);
    void setUniform(const char* name, int val);
    void setUniform(const char* name, bool val);
    void setUniform(const char* name, GLuint val);

    void findUniformLocations();
    void printActiveUniforms();
    void printActiveUniformBlocks();
    void printActiveAttribs();

    const char* getTypeString(GLenum type);
};

}  // namespace gdp1
