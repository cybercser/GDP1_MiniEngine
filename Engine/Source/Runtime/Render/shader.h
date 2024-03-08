#pragma once
// Adapted from https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition

#ifdef WIN32
#pragma warning(disable : 4290)
#endif

#include "common.h"
#include <stdexcept>
#include <glad/glad.h>

namespace gdp1 {

class GLSLProgramException : public std::runtime_error {
public:
    GLSLProgramException(const std::string& msg)
        : std::runtime_error(msg) {}
};

enum GLSLShaderType {
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    GEOMETRY = GL_GEOMETRY_SHADER,
    TESS_CONTROL = GL_TESS_CONTROL_SHADER,
    TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
    COMPUTE = GL_COMPUTE_SHADER
};

class Shader {
private:
    GLuint m_Handle;
    bool m_Linked;
    std::unordered_map<std::string, int> m_UniformLocations;

    void DetachAndDeleteShaderObjects();
    bool FileExists(const std::string& fileName);
    std::string GetExtension(const std::string& fileName);

public:
    Shader();
    ~Shader();

    // Make it non-copyable.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void CompileShader(const std::string& fileName);
    void CompileShader(const std::string& fileName, GLSLShaderType type);
    void CompileShader(const std::string& source, GLSLShaderType type, const std::string& fileName);

    void Link();
    void Validate();
    void Use();

    int GetHandle();
    bool IsLinked();

    void BindAttribLocation(GLuint location, const char* name);
    void BindFragDataLocation(GLuint location, const char* name);

    // std::string version of the following methods
    void SetUniform(const std::string& name, float x, float y, float z);
    void SetUniform(const std::string& name, const glm::vec2& v);
    void SetUniform(const std::string& name, const glm::vec3& v);
    void SetUniform(const std::string& name, const glm::vec4& v);
    void SetUniform(const std::string& name, const glm::mat4& m);
    void SetUniform(const std::string& name, const glm::mat3& m);
    void SetUniform(const std::string& name, float val);
    void SetUniform(const std::string& name, int val);
    void SetUniform(const std::string& name, bool val);
    void SetUniform(const std::string& name, GLuint val);

    void SetUniform(const char* name, float x, float y, float z);
    void SetUniform(const char* name, const glm::vec2& v);
    void SetUniform(const char* name, const glm::vec3& v);
    void SetUniform(const char* name, const glm::vec4& v);
    void SetUniform(const char* name, const glm::mat4& m);
    void SetUniform(const char* name, size_t size, GLboolean transpose, const glm::mat4& m);
    void SetUniform(const char* name, const glm::mat3& m);
    void SetUniform(const char* name, float val);
    void SetUniform(const char* name, int val);
    void SetUniform(const char* name, bool val);
    void SetUniform(const char* name, GLuint val);

    void FindUniformLocations();
    void PrintActiveUniforms();
    void PrintActiveUniformBlocks();
    void PrintActiveAttribs();

    inline GLint GetUniformLocation(const std::string& name);

    const char* GetTypeString(GLenum type);
};

}  // namespace gdp1
